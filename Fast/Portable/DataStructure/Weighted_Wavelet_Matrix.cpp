// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Weighted_Wavelet_Matrix.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/weighted_wavelet_matrix.md
//
// API一覧
// WeightedWaveletMatrix<Key,Sum>(keys,weights): 同じ長さのキー列と重み列。サイズはINT_MAX以下。
// sum(l,r): 元の位置区間[l,r)の重み和。O(1)。
// count_sum_less/less_equal(l,r,x): キーがx未満／以下の個数と重み和。O(1+B)。
// count_sum(l,r,lo,hi): lo<=key<hiの個数と重み和。lo>=hiはゼロ。O(1+B)。
// size()/empty(): 要素数／空か。O(1)。Bは異なるキーの個数σに対しceil(log2(σ))、空では0。
// 構築O(1+n log(n+1))時間、格納O(1+n(1+B))領域。キーは全順序、Sum{}は加法の単位元。
// 整数の和は型の幅で剰余演算。その他のSumは可換な加法と減法を備える。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_WEIGHTED_WAVELET_MATRIX_DEFINED
#define SIRKIT_WEIGHTED_WAVELET_MATRIX_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template <class S, bool Integral = std::is_integral_v<S> && !std::is_same_v<S, bool>>
struct WeightedWaveletStorage
{
    using Type = S;
};

template <class S>
struct WeightedWaveletStorage<S, true>
{
    using Type = std::make_unsigned_t<S>;
};

inline std::size_t CheckedWeightedWaveletSize(std::size_t count)
{
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("WeightedWaveletMatrix: too many values");
    }
    return count;
}
}

template <std::totally_ordered Key = std::int64_t, class Sum = std::int64_t>
    requires std::copyable<Key> && std::copyable<Sum> && std::default_initializable<Sum>
class WeightedWaveletMatrix
{
    static_assert(!std::is_same_v<Sum, bool>);

public:
    using CountSum = std::pair<std::size_t, Sum>;

    WeightedWaveletMatrix() = default;

    WeightedWaveletMatrix(std::span<const Key> keys, std::span<const Sum> weights)
    {
        if (keys.size() != weights.size())
        {
            throw std::invalid_argument("WeightedWaveletMatrix: mismatched sizes");
        }
        const auto n = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedWeightedWaveletSize(keys.size());
        if (n == 0)
        {
            return;
        }

        alphabet_.assign(keys.begin(), keys.end());
        std::sort(alphabet_.begin(), alphabet_.end());
        alphabet_.erase(std::unique(alphabet_.begin(), alphabet_.end()), alphabet_.end());

        std::vector<Stored> ordered(n);
        total_prefix_.reserve(n + 1);
        total_prefix_.emplace_back();
        for (std::size_t i = 0; i < n; ++i)
        {
            ordered[i] = Pack(weights[i]);
            total_prefix_.push_back(Add(total_prefix_.back(), ordered[i]));
        }
        const int height = std::bit_width(alphabet_.size() - 1);
        if (height == 0)
        {
            return;
        }

        std::vector<Index> ranks(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            ranks[i] = static_cast<Index>(std::lower_bound(alphabet_.begin(), alphabet_.end(), keys[i]) - alphabet_.begin());
        }
        std::vector<Index> next_ranks(n);
        std::vector<Stored> next_weights(n);
        levels_.reserve(height);
        for (int shift = height; shift-- > 0;)
        {
            levels_.emplace_back(n);
            auto &level = levels_.back();
            for (std::size_t i = 0; i < n; ++i)
            {
                if ((ranks[i] >> shift) & 1)
                {
                    level.words[i / 64] |= std::uint64_t{1} << (i % 64);
                }
            }
            for (std::size_t i = 0; i + 1 < level.words.size(); ++i)
            {
                level.ones_before[i + 1] = level.ones_before[i] + std::popcount(level.words[i]);
            }
            const std::size_t zeros = n - level.ones_before.back();
            level.zero_prefix.reserve(zeros + 1);
            level.zero_prefix.emplace_back();
            std::size_t zero = 0, one = zeros;
            for (std::size_t i = 0; i < n; ++i)
            {
                if ((ranks[i] >> shift) & 1)
                {
                    next_ranks[one] = ranks[i];
                    next_weights[one++] = ordered[i];
                }
                else
                {
                    level.zero_prefix.push_back(Add(level.zero_prefix.back(), ordered[i]));
                    next_ranks[zero] = ranks[i];
                    next_weights[zero++] = ordered[i];
                }
            }
            ranks.swap(next_ranks);
            ordered.swap(next_weights);
        }
    }

    WeightedWaveletMatrix(const WeightedWaveletMatrix &) = default;

    WeightedWaveletMatrix &operator=(const WeightedWaveletMatrix &other)
    {
        if (this != &other)
        {
            WeightedWaveletMatrix next(other);
            Swap(next);
        }
        return *this;
    }

    WeightedWaveletMatrix(WeightedWaveletMatrix &&other) noexcept
        : alphabet_(std::move(other.alphabet_)), total_prefix_(std::move(other.total_prefix_)),
          levels_(std::move(other.levels_))
    {
        other.alphabet_.clear();
        other.total_prefix_.clear();
        other.levels_.clear();
    }

    WeightedWaveletMatrix &operator=(WeightedWaveletMatrix &&other) noexcept
    {
        if (this != &other)
        {
            WeightedWaveletMatrix next(std::move(other));
            Swap(next);
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return total_prefix_.empty() ? 0 : total_prefix_.size() - 1;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]] Sum sum(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        return left == right ? Sum{} : Unpack(Subtract(total_prefix_[right], total_prefix_[left]));
    }

    [[nodiscard]] CountSum count_sum_less(std::size_t left, std::size_t right, const Key &upper) const
    {
        CheckRange(left, right);
        return Export(Below(left, right, LowerRank(upper)));
    }

    [[nodiscard]] CountSum count_sum_less_equal(std::size_t left, std::size_t right, const Key &upper) const
    {
        CheckRange(left, right);
        const auto limit = std::upper_bound(alphabet_.begin(), alphabet_.end(), upper) - alphabet_.begin();
        return Export(Below(left, right, static_cast<std::size_t>(limit)));
    }

    [[nodiscard]] CountSum count_sum(std::size_t left, std::size_t right, const Key &lower, const Key &upper) const
    {
        CheckRange(left, right);
        if (!(lower < upper))
        {
            return {};
        }
        const auto low = Below(left, right, LowerRank(lower));
        const auto high = Below(left, right, LowerRank(upper));
        return {high.first - low.first, Unpack(Subtract(high.second, low.second))};
    }

private:
    using Stored = typename /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::WeightedWaveletStorage<Sum>::Type;
    using Index = std::uint32_t;
    using Partial = std::pair<std::size_t, Stored>;

    struct Level
    {
        // nが64の倍数の場合にもrank(n)で読めるように末尾に0語を置く。
        explicit Level(std::size_t n) : words((n + 63) / 64 + 1), ones_before(words.size())
        {
        }

        [[nodiscard]] std::size_t rank(std::size_t index) const noexcept
        {
            const auto word = index / 64;
            const auto mask = (std::uint64_t{1} << (index % 64)) - 1;
            return ones_before[word] + std::popcount(words[word] & mask);
        }

        std::vector<std::uint64_t> words;
        std::vector<Index> ones_before;
        std::vector<Stored> zero_prefix;
    };

    std::vector<Key> alphabet_;
    std::vector<Stored> total_prefix_;
    std::vector<Level> levels_;

    void Swap(WeightedWaveletMatrix &other) noexcept
    {
        alphabet_.swap(other.alphabet_);
        total_prefix_.swap(other.total_prefix_);
        levels_.swap(other.levels_);
    }

    void CheckRange(std::size_t left, std::size_t right) const
    {
        (void)left;
        (void)right;
    }

    static Stored Pack(const Sum &value)
    {
        return static_cast<Stored>(value);
    }

    static Sum Unpack(const Stored &value)
    {
        if constexpr (std::is_integral_v<Sum> && std::is_signed_v<Sum>)
        {
            return std::bit_cast<Sum>(value);
        }
        else
        {
            return value;
        }
    }

    static Stored Add(const Stored &a, const Stored &b)
    {
        return static_cast<Stored>(a + b);
    }

    static Stored Subtract(const Stored &a, const Stored &b)
    {
        return static_cast<Stored>(a - b);
    }

    [[nodiscard]] std::size_t LowerRank(const Key &key) const
    {
        return static_cast<std::size_t>(std::lower_bound(alphabet_.begin(), alphabet_.end(), key) - alphabet_.begin());
    }

    static CountSum Export(const Partial &value)
    {
        return {value.first, Unpack(value.second)};
    }

    Partial Below(std::size_t left, std::size_t right, std::size_t limit) const
    {
        if (left == right || limit == 0)
        {
            return {};
        }
        if (limit == alphabet_.size())
        {
            return {right - left, Subtract(total_prefix_[right], total_prefix_[left])};
        }

        Partial result{};
        auto level_index = levels_.size();
        for (const auto &level : levels_)
        {
            const auto shift = --level_index;
            const auto left_one = level.rank(left);
            const auto right_one = level.rank(right);
            const auto left_zero = left - left_one;
            const auto right_zero = right - right_one;
            if ((limit >> shift) & 1)
            {
                result.first += right_zero - left_zero;
                result.second = Add(result.second, Subtract(level.zero_prefix[right_zero], level.zero_prefix[left_zero]));
                const auto zeros = level.zero_prefix.size() - 1;
                left = zeros + left_one;
                right = zeros + right_one;
            }
            else
            {
                left = left_zero;
                right = right_zero;
            }
            if (left == right)
            {
                break;
            }
        }
        return result;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
