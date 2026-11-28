// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Persistent_Binary_Trie.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_binary_trie.md
//
// API一覧
// PersistentBinaryTrie<T,MAX_LOG>(expected_updates=0): 空の多重集合。更新回数の見込みに応じて容量を確保する。
// Tはboolを除く符号なし整数。MAX_LOGは最高位ビット番号で既定値はTのビット数-1。B=MAX_LOG+1。
// Version{}: 空集合の版。各版は作成した木で使う。キー・マスクはBビット以内。
// size(version): 多重集合の要素数を返す。O(1)。count(version,key): キーの個数を返す。O(B)。
// insert(version,key): キーを1個追加した版を返す。元の版は保持する。償却O(B)時間・追加O(B)領域。
// erase(version,key): キーを1個除いた版を返す。存在しない場合nullopt。償却O(B)時間・追加O(B)領域。
// kth_xor(before,after,k,mask=0): 差分中でkey^maskが0始まりk番目の値。なければnullopt。O(B)。
// count_xor_less(before,after,bound,mask=0): 差分中でkey^mask<boundとなる要素数。O(B)。
// beforeはafterの部分多重集合。insertで作った接頭辞の版を使えば添字区間を照会できる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_BINARY_TRIE_DEFINED
#define SIRKIT_PERSISTENT_BINARY_TRIE_DEFINED

template <std::unsigned_integral T = std::uint32_t, int MAX_LOG = std::numeric_limits<T>::digits - 1>
    requires(!std::same_as<T, bool> && MAX_LOG >= 0 && MAX_LOG < std::numeric_limits<T>::digits)
class PersistentBinaryTrie
{
    using Index = std::uint32_t;
    static constexpr std::size_t kPathNodes = MAX_LOG + 2;

public:
    class Version
    {
        friend class PersistentBinaryTrie;

    public:
        Version() = default;

    private:
        explicit Version(Index root) : root_(root)
        {
        }

        Index root_ = 0;
    };

    explicit PersistentBinaryTrie(std::size_t expected_updates = 0)
    {
        Prepare(expected_updates);
    }

    std::uint32_t size(Version version) const
    {
        CheckVersion(version);
        return Get(version.root_).count;
    }

    std::uint32_t count(Version version, T key) const
    {
        CheckVersion(version);
        CheckKey(key);
        Index node = version.root_;
        for (int bit = MAX_LOG; bit >= 0 && node != 0; --bit)
        {
            node = Get(node).children[Bit(key, bit)];
        }
        return Get(node).count;
    }

    Version insert(Version version, T key)
    {
        CheckVersion(version);
        CheckKey(key);
        Prepare(1);
        return Version(Change<true>(version.root_, key, MAX_LOG));
    }

    std::optional<Version> erase(Version version, T key)
    {
        if (count(version, key) == 0)
        {
            return std::nullopt;
        }
        Prepare(1);
        return Version(Change<false>(version.root_, key, MAX_LOG));
    }

    std::optional<T> kth_xor(Version before, Version after, std::uint32_t k, T mask = 0) const
    {
        CheckInterval(before, after);
        CheckKey(mask);
        if (k >= Difference(before.root_, after.root_))
        {
            return std::nullopt;
        }
        T result = 0;
        Index left = before.root_;
        Index right = after.root_;
        for (int bit = MAX_LOG; bit >= 0; --bit)
        {
            const auto side = Bit(mask, bit);
            const auto count_zero = Difference(Get(left).children[side], Get(right).children[side]);
            std::size_t direction = side;
            if (k >= count_zero)
            {
                k -= count_zero;
                direction ^= 1;
                result |= T{1} << bit;
            }
            left = Get(left).children[direction];
            right = Get(right).children[direction];
        }
        return result;
    }

    std::uint32_t count_xor_less(Version before, Version after, T bound, T mask = 0) const
    {
        CheckInterval(before, after);
        CheckKey(mask);
        Index left = before.root_;
        Index right = after.root_;
        if (!Fits(bound))
        {
            return Difference(left, right);
        }
        std::uint32_t smaller = 0;
        for (int bit = MAX_LOG; bit >= 0 && left != right; --bit)
        {
            const auto zero = Bit(mask, bit);
            const auto allows_zero = Bit(bound, bit);
            if (allows_zero)
            {
                smaller += Difference(Get(left).children[zero], Get(right).children[zero]);
            }
            const auto direction = zero ^ allows_zero;
            left = Get(left).children[direction];
            right = Get(right).children[direction];
        }
        return smaller;
    }

private:
    struct Node
    {
        std::array<Index, 2> children{};
        Index count{};
    };

    static constexpr Node kEmpty{};
    std::vector<Node> nodes_;

    const Node &Get(Index id) const
    {
        return id == 0 ? kEmpty : nodes_[id - 1];
    }

    static std::size_t Bit(T key, int bit)
    {
        return (key >> bit) & T{1};
    }

    static bool Fits(T key)
    {
        return (key >> MAX_LOG) <= T{1};
    }

    void Prepare(std::size_t updates)
    {
        const auto limit = std::min<std::size_t>(std::numeric_limits<Index>::max(), nodes_.max_size());
        if (updates > (limit - nodes_.size()) / kPathNodes)
        {
            throw std::length_error("PersistentBinaryTrie: too many nodes");
        }
        const auto need = nodes_.size() + updates * kPathNodes;
        const auto capacity = nodes_.capacity();
        if (need > capacity)
        {
            const auto growth = std::min(capacity / 2, limit - capacity);
            nodes_.reserve(std::max(need, capacity + growth));
        }
    }

    template <bool Add>
    Index Change(Index id, T key, int bit)
    {
        Node copy = Get(id);
        if constexpr (Add)
        {
            ++copy.count;
        }
        else
        {
            if (--copy.count == 0)
            {
                return 0;
            }
        }
        if (bit >= 0)
        {
            const auto side = Bit(key, bit);
            copy.children[side] = Change<Add>(copy.children[side], key, bit - 1);
        }
        nodes_.push_back(copy);
        return static_cast<Index>(nodes_.size());
    }

    std::uint32_t Difference(Index before, Index after) const
    {
        return Get(after).count - Get(before).count;
    }

    void CheckVersion([[maybe_unused]] Version version) const
    {
    }

    static void CheckKey([[maybe_unused]] T key)
    {
    }

    void CheckInterval([[maybe_unused]] Version before, [[maybe_unused]] Version after) const
    {
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
