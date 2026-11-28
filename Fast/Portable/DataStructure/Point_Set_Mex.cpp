// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Point_Set_Mex.cpp
// 参考: https://atcoder.jp/contests/abc330/editorial/7752
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/point_set_mex.md
//
// API一覧
// PointSetMex<T=int>(values): 整数列を所有して準備する。長さnは固定。O(n+1)時間・領域。
// PointSetMex<T>(): 空列を用意する。O(1)。コピーは独立。ムーブ元は空列。
// size(): 要素数。get(i): 0始まり添字iの値。mex(): 列にない最小の非負整数。各O(1)。
// set(i,value): 位置iを更新。O(1+log_64(n+1))。負数とnより大きい値も保持し、mexの計算から外す。
// get/setはi<size()が前提。検査有効時の範囲外はstd::out_of_range。
//
// [同梱の階層化ビット集合]
// HierarchicalBitset(n=0): 0..n-1を空集合で初期化。O(1+n/64)時間・ワード。
// HierarchicalBitset(bits): 0/1文字列の各位置を初期状態にする。O(n+1)時間。
// insert(p)/erase(p): 位置pを追加／削除。変更時true。O(1+log_64(n+1))。
// contains(p): 位置pを含むか。O(1)。3操作の位置は0<=p<n。
// next(p): p以上の最小の要素。prev(p): p以下の最大の要素。なければnullopt。O(1+log_64(n+1))。
// size()/count()/empty(): 添字空間長・集合の要素数・集合が空か。O(1)。
// 各64ビット語の非零を次の層で持つ。全体でO(1+n/64)ワード。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>
#include <concepts>
#include <string>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HIERARCHICAL_BITSET_DEFINED
#define SIRKIT_HIERARCHICAL_BITSET_DEFINED

class HierarchicalBitset
{
public:
    explicit HierarchicalBitset(std::size_t n = 0) : length_(n)
    {
        for (auto bits = length_; bits != 0;)
        {
            const auto words = ((bits - 1) >> 6) + 1;
            levels_.emplace_back(words, 0);
            if (words == 1)
            {
                break;
            }
            bits = words;
        }
    }

    explicit HierarchicalBitset(std::string_view bits) : HierarchicalBitset(bits.size())
    {
        for (std::size_t i = 0; i < length_; ++i)
        {
            const char digit = bits[i];
            if (digit == '1')
            {
                SetBit(levels_[0], i);
                ++count_;
            }
        }
        for (std::size_t level = 1; level < levels_.size(); ++level)
        {
            const auto &lower = levels_[level - 1];
            auto &current = levels_[level];
            for (std::size_t word = 0; word < lower.size(); ++word)
            {
                if (lower[word] != 0)
                {
                    SetBit(current, word);
                }
            }
        }
    }

    HierarchicalBitset(const HierarchicalBitset &) = default;

    HierarchicalBitset(HierarchicalBitset &&other) noexcept
        : levels_(std::move(other.levels_)), length_(std::exchange(other.length_, 0)),
          count_(std::exchange(other.count_, 0))
    {
        other.levels_.clear();
    }

    HierarchicalBitset &operator=(HierarchicalBitset other) noexcept
    {
        levels_.swap(other.levels_);
        std::swap(length_, other.length_);
        std::swap(count_, other.count_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return length_;
    }

    [[nodiscard]] std::size_t count() const noexcept
    {
        return count_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return count_ == 0;
    }

    [[nodiscard]] bool contains(std::size_t position) const
    {
        CheckPosition(position);
        return (levels_[0][position >> 6] & Mask(position)) != 0;
    }

    bool insert(std::size_t position)
    {
        return Change<true>(position);
    }

    bool erase(std::size_t position)
    {
        return Change<false>(position);
    }

    [[nodiscard]] std::optional<std::size_t> next(std::size_t from) const noexcept
    {
        if (from >= length_)
        {
            return std::nullopt;
        }
        return Find<true>(from);
    }

    [[nodiscard]] std::optional<std::size_t> prev(std::size_t from) const noexcept
    {
        if (length_ == 0)
        {
            return std::nullopt;
        }
        return Find<false>(std::min(from, length_ - 1));
    }

private:
    using Word = std::uint64_t;
    static constexpr Word kAll = ~Word{0};
    std::vector<std::vector<Word>> levels_;
    std::size_t length_, count_ = 0;

    void CheckPosition([[maybe_unused]] std::size_t position) const
    {
    }

    static Word Mask(std::size_t position) noexcept
    {
        return Word{1} << (position & 63);
    }

    static void SetBit(std::vector<Word> &words, std::size_t position) noexcept
    {
        words[position >> 6] |= Mask(position);
    }

    template <bool Present>
    bool Change(std::size_t position)
    {
        if (contains(position) == Present)
        {
            return false;
        }
        if constexpr (Present)
        {
            ++count_;
        }
        else
        {
            --count_;
        }
        for (auto &level : levels_)
        {
            auto &word = level[position >> 6];
            if constexpr (Present)
            {
                const auto before = word;
                word |= Mask(position);
                if (before != 0)
                {
                    break;
                }
            }
            else
            {
                word &= ~Mask(position);
                if (word != 0)
                {
                    break;
                }
            }
            position >>= 6;
        }
        return true;
    }

    template <bool Forward>
    static unsigned SelectBit(Word word) noexcept
    {
        if constexpr (Forward)
        {
            return static_cast<unsigned>(std::countr_zero(word));
        }
        else
        {
            return static_cast<unsigned>(std::bit_width(word) - 1);
        }
    }

    template <bool Forward>
    std::size_t Descend(std::size_t level, std::size_t position) const noexcept
    {
        while (level > 0)
        {
            const auto word = levels_[--level][position];
            position = (position << 6) | SelectBit<Forward>(word);
        }
        return position;
    }

    template <bool Forward>
    std::optional<std::size_t> Find(std::size_t position) const noexcept
    {
        for (std::size_t level = 0; level < levels_.size(); ++level)
        {
            const auto block = position >> 6;
            const auto &words = levels_[level];
            if constexpr (Forward)
            {
                // 上位層で最後の語の直後を参照する場合がある。
                if (block >= words.size())
                {
                    return std::nullopt;
                }
            }
            const auto bit = position & 63;
            const auto mask = Forward ? kAll << bit : kAll >> (63 - bit);
            const auto available = words[block] & mask;
            if (available != 0)
            {
                const auto found = (block << 6) | SelectBit<Forward>(available);
                return Descend<Forward>(level, found);
            }
            if constexpr (Forward)
            {
                position = block + 1;
            }
            else
            {
                if (block == 0)
                {
                    return std::nullopt;
                }
                position = block - 1;
            }
        }
        return std::nullopt;
    }
};

#endif

#ifndef SIRKIT_POINT_SET_MEX_DEFINED
#define SIRKIT_POINT_SET_MEX_DEFINED

template <std::integral T = int>
    requires (sizeof(T) <= sizeof(std::uintmax_t))
class PointSetMex
{
    std::vector<T> values_;
    std::vector<std::size_t> counts_;
    HierarchicalBitset missing_;
    std::size_t mex_ = 0;

    [[nodiscard]] bool tracked(T value) const noexcept
    {
        if constexpr (std::signed_integral<T>)
        {
            if (value < 0)
            {
                return false;
            }
        }
        return static_cast<std::uintmax_t>(value) < counts_.size();
    }

    void check_position([[maybe_unused]] std::size_t position) const
    {
    }

public:
    PointSetMex() = default;

    explicit PointSetMex(std::vector<T> values)
        : values_(std::move(values)), counts_(values_.size() + 1)
    {
        for (T value : values_)
        {
            if (tracked(value))
            {
                ++counts_[static_cast<std::size_t>(value)];
            }
        }
        std::string marks(counts_.size(), '0');
        for (std::size_t i = 0; i < counts_.size(); ++i)
        {
            marks[i] = counts_[i] == 0 ? '1' : '0';
        }
        missing_ = HierarchicalBitset(marks);
        mex_ = *missing_.next(0);
    }

    PointSetMex(const PointSetMex &) = default;

    PointSetMex(PointSetMex &&other) noexcept
        : values_(std::move(other.values_)), counts_(std::move(other.counts_)), missing_(std::move(other.missing_)),
          mex_(std::exchange(other.mex_, 0))
    {
        other.values_.clear();
        other.counts_.clear();
    }

    PointSetMex &operator=(PointSetMex other) noexcept
    {
        values_.swap(other.values_);
        counts_.swap(other.counts_);
        std::swap(missing_, other.missing_);
        std::swap(mex_, other.mex_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return values_.size();
    }

    [[nodiscard]] T get(std::size_t position) const
    {
        check_position(position);
        return values_[position];
    }

    [[nodiscard]] std::size_t mex() const noexcept
    {
        return mex_;
    }

    void set(std::size_t position, T value)
    {
        check_position(position);
        const T previous = values_[position];
        if (previous == value)
        {
            return;
        }
        const auto removed = static_cast<std::size_t>(previous);
        if (tracked(previous) && --counts_[removed] == 0)
        {
            missing_.insert(removed);
            mex_ = std::min(mex_, removed);
        }
        const auto added = static_cast<std::size_t>(value);
        if (tracked(value) && counts_[added]++ == 0)
        {
            missing_.erase(added);
            if (mex_ == added)
            {
                // n個の値で0..nのすべてを埋めることはできない。
                mex_ = *missing_.next(added + 1);
            }
        }
        values_[position] = value;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
