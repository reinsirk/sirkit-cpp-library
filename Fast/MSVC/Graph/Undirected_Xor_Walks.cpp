// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Undirected_Xor_Walks.cpp
// 参考（数学的構成）: https://atcoder.jp/contests/abc451/editorial/18047/print
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/undirected_xor_walks.md
//
// API一覧
// XorWalkEdge{from,to,weight}: 0始まりの両端点とuint64_tの辺重み。
// UndirectedXorWalks(n,edges): 無向グラフの連結成分とXOR基底を作る。O(64(n+m))時間、O(n+m+64c)領域。
//   cはXORが非零となる閉じた歩道を持つ連結成分の数。
// min_xor(u,v): uからvへの歩道の最小XOR。到達不能ならnullopt。O(1)。
// max_xor(u,v): 最大XOR。到達不能ならnullopt。O(64)。
// attainable(u,v,x): XORがxとなる歩道が存在するか。O(64)。
// count_pairs_at_most(x): 最小XORがx以下になる頂点組u<vの数。O(n(log(n+1)+64))時間、追加O(n)領域。
//   歩道は頂点・辺の再訪と、同じ頂点での空の歩道を許す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_XOR_BASIS_DEFINED
#define SIRKIT_XOR_BASIS_DEFINED
class XorBasis {
    std::array<std::uint64_t, 64> basis_{};
    unsigned rank_ = 0;
    bool dependent_ = false;
public:
    bool insert(std::uint64_t value) noexcept {
        for (unsigned bit = 64; bit != 0;) {
            --bit;
            if (((value >> bit) & 1) == 0) continue;
            if (basis_[bit] != 0) value ^= basis_[bit];
            else { basis_[bit] = value; ++rank_; return true; }
        }
        dependent_ = true;
        return false;
    }
    void merge(const XorBasis& other) noexcept {
        // 自己結合でも非零基底の値は変化せず、従属性だけが増える。
        for (auto value : other.basis_) if (value != 0) insert(value);
        dependent_ = dependent_ || other.dependent_;
    }
    bool contains(std::uint64_t value) const noexcept {
        for (unsigned bit = 64; bit != 0;) {
            --bit;
            if (((value >> bit) & 1) != 0) value ^= basis_[bit];
        }
        return value == 0;
    }
    bool contains_nonempty(std::uint64_t value) const noexcept { return value == 0 ? dependent_ : contains(value); }
    std::uint64_t max_xor(std::uint64_t seed = 0) const noexcept {
        for (unsigned bit = 64; bit != 0;) { --bit; seed = std::max(seed, seed ^ basis_[bit]); }
        return seed;
    }
    std::uint64_t min_xor(std::uint64_t seed = 0) const noexcept {
        for (unsigned bit = 64; bit != 0;) { --bit; seed = std::min(seed, seed ^ basis_[bit]); }
        return seed;
    }
    std::optional<std::uint64_t> kth(std::uint64_t k) const noexcept {
        if (rank_ < 64 && k >= (std::uint64_t{1} << rank_)) return std::nullopt;
        auto reduced = basis_;
        for (unsigned bit = 0; bit < 64; ++bit) {
            if (reduced[bit] == 0) continue;
            for (unsigned lower = 0; lower < bit; ++lower)
                if (((reduced[bit] >> lower) & 1) != 0) reduced[bit] ^= reduced[lower];
        }
        std::uint64_t result = 0;
        unsigned position = 0;
        for (auto value : reduced) {
            if (value == 0) continue;
            if (((k >> position) & 1) != 0) result ^= value;
            ++position;
        }
        return result;
    }
    unsigned rank() const noexcept { return rank_; }
    bool has_nonempty_zero() const noexcept { return dependent_; }
    void clear() noexcept { basis_.fill(0); rank_ = 0; dependent_ = false; }
    void swap(XorBasis& other) noexcept { basis_.swap(other.basis_); std::swap(rank_, other.rank_); std::swap(dependent_, other.dependent_); }
};
#endif

#ifndef SIRKIT_PAIR_XOR_ORDER_DEFINED
#define SIRKIT_PAIR_XOR_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{

inline void CheckPairXorSize(std::size_t size)
{
    if (size > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("PairXorOrder: too many values");
    }
}

}

template <std::unsigned_integral T = std::uint64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class PairXorOrder
{
    struct Range
    {
        std::uint32_t begin, end;

        [[nodiscard]] std::uint64_t size() const
        {
            return end - begin;
        }
    };

    struct Pair
    {
        Range first, second;
    };

    struct Bucket
    {
        std::vector<Pair> pairs;
        std::uint64_t count = 0;

        void add(Range first, Range second)
        {
            if (first.size() == 0 || second.size() == 0)
            {
                return;
            }
            const auto ways = first.begin == second.begin
                                  ? PairCount(first.size())
                                  : first.size() * second.size();
            if (ways != 0)
            {
                pairs.push_back({first, second});
                count += ways;
            }
        }

        void clear()
        {
            pairs.clear();
            count = 0;
        }
    };

public:
    explicit PairXorOrder(std::vector<T> values) : values_(std::move(values))
    {
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckPairXorSize(values_.size());
        std::sort(values_.begin(), values_.end());
    }

    [[nodiscard]] std::uint64_t pair_count() const noexcept
    {
        return PairCount(values_.size());
    }

    [[nodiscard]] T kth(std::uint64_t k) const
    {
        T value = 0;
        Search(BitWidth(), [&](T bit, std::uint64_t same)
               {
                   if (k < same)
                   {
                       return false;
                   }
                   k -= same;
                   value |= bit;
                   return true;
               });
        return value;
    }

    [[nodiscard]] std::uint64_t count_less(T bound) const
    {
        if (pair_count() == 0 || bound == 0)
        {
            return 0;
        }
        const int bits = BitWidth();
        if (std::bit_width(bound) > bits)
        {
            return pair_count();
        }
        std::uint64_t count = 0;
        Search(bits, [&](T bit, std::uint64_t same)
               {
                   if ((bound & bit) == 0)
                   {
                       return false;
                   }
                   count += same;
                   return true;
               });
        return count;
    }

private:
    [[nodiscard]] static std::uint64_t PairCount(std::uint64_t size) noexcept
    {
        return size * (size - 1) / 2;
    }

    [[nodiscard]] int BitWidth() const
    {
        return std::bit_width(static_cast<T>(values_.front() ^ values_.back()));
    }

    [[nodiscard]] std::pair<Range, Range> Split(Range range, T bit) const
    {
        auto it = std::partition_point(values_.begin() + range.begin, values_.begin() + range.end,
                                       [&](T value)
                                       {
                                           return (value & bit) == 0;
                                       });
        const auto middle = static_cast<std::uint32_t>(it - values_.begin());
        return {{range.begin, middle}, {middle, range.end}};
    }

    template <class ChooseOne>
    void Search(int bits, ChooseOne choose_one) const
    {
        const Range all{0, static_cast<std::uint32_t>(values_.size())};
        Bucket current, same, different;
        current.add(all, all);
        for (int level = bits - 1; level >= 0 && !current.pairs.empty(); --level)
        {
            const auto bit = static_cast<T>(T{1} << level);
            same.clear();
            different.clear();
            for (const auto &[first, second] : current.pairs)
            {
                const auto [a0, a1] = Split(first, bit);
                if (first.begin == second.begin)
                {
                    same.add(a0, a0);
                    same.add(a1, a1);
                    different.add(a0, a1);
                }
                else
                {
                    const auto [b0, b1] = Split(second, bit);
                    same.add(a0, b0);
                    same.add(a1, b1);
                    different.add(a0, b1);
                    different.add(a1, b0);
                }
            }
            Bucket &next = choose_one(bit, same.count) ? different : same;
            current.pairs.swap(next.pairs);
        }
    }

    std::vector<T> values_;
};

#endif

#ifndef SIRKIT_XOR_WALK_EDGE_DEFINED
#define SIRKIT_XOR_WALK_EDGE_DEFINED

struct XorWalkEdge
{
    int from, to;
    std::uint64_t weight;
};

namespace detail
{
inline void check_xor_walk_vertex(int vertex, int n)
{
    (void)vertex;
    (void)n;
}
} // namespace detail

#endif

#ifndef SIRKIT_UNDIRECTED_XOR_WALKS_DEFINED
#define SIRKIT_UNDIRECTED_XOR_WALKS_DEFINED

class UndirectedXorWalks
{
public:
    UndirectedXorWalks(int n, std::span<const XorWalkEdge> edges)
    {
        std::vector<std::vector<std::size_t>> adjacent(n);
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            const auto &[u, v, weight] = edges[id];
            detail::check_xor_walk_vertex(u, n);
            detail::check_xor_walk_vertex(v, n);
            adjacent[u].push_back(id);
            if (u != v)
            {
                adjacent[v].push_back(id);
            }
        }

        component_.assign(n, -1);
        reduced_.resize(n);
        vertices_.reserve(n);
        for (int vertex = 0; vertex < n; ++vertex)
        {
            if (component_[vertex] != -1)
            {
                continue;
            }
            const int component = static_cast<int>(basis_index_.size());
            const auto begin = vertices_.size();
            vertices_.push_back(vertex);
            component_[vertex] = component;
            for (auto i = begin; i < vertices_.size(); ++i)
            {
                const int u = vertices_[i];
                for (const auto id : adjacent[u])
                {
                    const auto &edge = edges[id];
                    const int v = edge.from == u ? edge.to : edge.from;
                    if (component_[v] == -1)
                    {
                        component_[v] = component;
                        reduced_[v] = reduced_[u] ^ edge.weight;
                        vertices_.push_back(v);
                    }
                }
            }

            // 全域木を使って辺ごとに閉じた歩道のXORを作る。
            XorBasis basis;
            for (auto i = begin; i < vertices_.size(); ++i)
            {
                const int u = vertices_[i];
                for (const auto id : adjacent[u])
                {
                    const auto &edge = edges[id];
                    if (edge.from == u)
                    {
                        const auto cycle = reduced_[u] ^ edge.weight ^ reduced_[edge.to];
                        if (cycle != 0)
                        {
                            basis.insert(cycle);
                        }
                    }
                }
            }
            int index = -1;
            if (basis.rank() != 0)
            {
                index = static_cast<int>(bases_.size());
                bases_.push_back(basis);
                // 基底のピボットを消す写像はXORに関して線形。
                for (auto i = begin; i < vertices_.size(); ++i)
                {
                    const int u = vertices_[i];
                    reduced_[u] = basis.min_xor(reduced_[u]);
                }
            }
            basis_index_.push_back(index);
            offsets_.push_back(vertices_.size());
        }
    }

    [[nodiscard]] std::optional<std::uint64_t> min_xor(int u, int v) const
    {
        if (!connected(u, v))
        {
            return std::nullopt;
        }
        return reduced_[u] ^ reduced_[v];
    }

    [[nodiscard]] std::optional<std::uint64_t> max_xor(int u, int v) const
    {
        const auto base = min_xor(u, v);
        if (!base)
        {
            return std::nullopt;
        }
        const int index = basis_index_[component_[u]];
        return index < 0 ? *base : bases_[index].max_xor(*base);
    }

    [[nodiscard]] bool attainable(int u, int v, std::uint64_t value) const
    {
        const auto base = min_xor(u, v);
        if (!base)
        {
            return false;
        }
        const int index = basis_index_[component_[u]];
        return index < 0 ? *base == value : bases_[index].contains(*base ^ value);
    }

    [[nodiscard]] std::uint64_t count_pairs_at_most(std::uint64_t bound) const
    {
        std::uint64_t count = 0;
        for (std::size_t c = 0; c < basis_index_.size(); ++c)
        {
            const auto begin = offsets_[c];
            const std::uint64_t length = offsets_[c + 1] - begin;
            if (length < 2)
            {
                continue;
            }
            if (bound == std::numeric_limits<std::uint64_t>::max())
            {
                count += length * (length - 1) / 2;
                continue;
            }
            std::vector<std::uint64_t> values;
            values.reserve(length);
            for (auto i = begin; i < offsets_[c + 1]; ++i)
            {
                values.push_back(reduced_[vertices_[i]]);
            }
            count += PairXorOrder<>(std::move(values)).count_less(bound + 1);
        }
        return count;
    }

private:
    [[nodiscard]] bool connected(int u, int v) const
    {
        const auto n = static_cast<int>(component_.size());
        detail::check_xor_walk_vertex(u, n);
        detail::check_xor_walk_vertex(v, n);
        return component_[u] == component_[v];
    }

    std::vector<int> component_, basis_index_, vertices_;
    std::vector<std::size_t> offsets_{0};
    std::vector<std::uint64_t> reduced_;
    std::vector<XorBasis> bases_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
