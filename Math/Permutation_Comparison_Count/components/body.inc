#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SIRKIT_PERMUTATION_COMPARISON_COUNT_DEFINED
#define SIRKIT_PERMUTATION_COMPARISON_COUNT_DEFINED

namespace detail
{
template <std::uint32_t Prime>
class ComparisonPermutationCounter
{
    static constexpr std::size_t kDirectCuts = 48;
    using Convolution = std::conditional_t<Prime == 998244353, NTT<>, CRTConvolution>;

public:
    explicit ComparisonPermutationCounter(std::size_t length) : inverse_factorials_(length + 1)
    {
        for (std::size_t i = 2; i <= length; ++i)
        {
            factorial_ = Multiply(factorial_, static_cast<std::uint32_t>(i));
        }
        inverse_factorials_[length] = Power(factorial_, Prime - 2);
        for (auto i = length; i != 0; --i)
        {
            inverse_factorials_[i - 1] = Multiply(inverse_factorials_[i], static_cast<std::uint32_t>(i));
        }
    }

    [[nodiscard]] std::uint32_t Count(std::string_view comparisons)
    {
        auto answer = factorial_;
        for (;;)
        {
            const auto cut = comparisons.find('?');
            answer = Multiply(answer, NormalizedCount(comparisons.substr(0, cut)));
            if (cut == std::string_view::npos)
            {
                return answer;
            }
            comparisons.remove_prefix(cut + 1);
        }
    }

private:
    static std::uint32_t Add(std::uint32_t left, std::uint32_t right)
    {
        const auto total = std::uint64_t{left} + right;
        return static_cast<std::uint32_t>(total >= Prime ? total - Prime : total);
    }

    static std::uint32_t Multiply(std::uint32_t left, std::uint32_t right)
    {
        return static_cast<std::uint32_t>(std::uint64_t{left} * right % Prime);
    }

    static std::uint32_t Power(std::uint32_t value, std::uint32_t exponent)
    {
        std::uint32_t answer = 1;
        for (; exponent != 0; exponent >>= 1)
        {
            if ((exponent & 1u) != 0)
            {
                answer = Multiply(answer, value);
            }
            value = Multiply(value, value);
        }
        return answer;
    }

    [[nodiscard]] std::uint32_t NormalizedCount(std::string_view comparisons)
    {
        const auto greater_count = static_cast<std::size_t>(std::ranges::count(comparisons, '>'));
        const auto smaller_count = comparisons.size() - greater_count;
        const auto length = comparisons.size() + 1;
        if (greater_count == 0 || smaller_count == 0)
        {
            return inverse_factorials_[length];
        }
        const char descent = greater_count <= smaller_count ? '>' : '<'; // 値を反転して指定の少ない方を降順にする。
        std::vector<std::size_t> cuts;
        cuts.reserve(std::min(greater_count, smaller_count) + 2);
        cuts.push_back(0);
        for (std::size_t i = 0; i < comparisons.size(); ++i)
        {
            if (comparisons[i] == descent)
            {
                cuts.push_back(i + 1);
            }
        }
        cuts.push_back(length);

        std::vector<std::uint32_t> weights(cuts.size());
        weights[0] = 1;
        Accumulate(cuts, weights, 0, cuts.size());
        if ((cuts.size() & 1u) != 0 && weights.back() != 0)
        {
            return Prime - weights.back(); // 包除全体に、指定された降順の個数に応じた符号を付ける。
        }
        return weights.back();
    }

    void Accumulate(const std::vector<std::size_t> &cuts, std::vector<std::uint32_t> &weights,
                    std::size_t first, std::size_t last)
    {
        if (last - first <= kDirectCuts)
        {
            for (auto i = first; i < last; ++i)
            {
                auto &weight = weights[i];
                if (i != 0 && i + 1 != cuts.size() && weight != 0)
                {
                    weight = Prime - weight; // 途中の区切りごとに包除の符号を反転する。
                }
                for (auto j = i + 1; j < last; ++j)
                {
                    weights[j] = Add(weights[j], Multiply(weight, inverse_factorials_[cuts[j] - cuts[i]]));
                }
            }
            return;
        }
        const auto middle = first + (last - first) / 2;
        Accumulate(cuts, weights, first, middle);
        Propagate(cuts, weights, first, middle, last);
        Accumulate(cuts, weights, middle, last);
    }

    void Propagate(const std::vector<std::size_t> &cuts, std::vector<std::uint32_t> &weights,
                   std::size_t first, std::size_t middle, std::size_t last)
    {
        const auto origin = cuts[first];
        std::vector<std::uint32_t> left(cuts[middle - 1] - origin + 1);
        for (auto i = first; i < middle; ++i)
        {
            left[cuts[i] - origin] = weights[i];
        }
        const auto kernel_length = static_cast<std::ptrdiff_t>(cuts[last - 1] - origin + 1);
        std::vector<std::uint32_t> kernel(inverse_factorials_.begin(), inverse_factorials_.begin() + kernel_length);
        std::vector<std::uint32_t> product;
        if constexpr (Prime == 998244353)
        {
            product = convolution_.convolution_normalized(std::move(left), std::move(kernel));
        }
        else
        {
            product = convolution_.modular(left, kernel, Prime);
        }
        for (auto j = middle; j < last; ++j)
        {
            weights[j] = Add(weights[j], product[cuts[j] - origin]);
        }
    }

    Convolution convolution_;
    std::vector<std::uint32_t> inverse_factorials_;
    std::uint32_t factorial_ = 1;
};
} // namespace detail

template <std::uint32_t Prime = 998244353>
[[nodiscard]] std::uint32_t count_permutations_by_comparisons(std::string_view comparisons)
{
    static_assert(detail::ntt_prime(Prime), "count_permutations_by_comparisons: Prime must be prime");
    constexpr auto kMaxLength = (std::size_t{1} << 22) - 1;
    if (comparisons.size() >= std::min<std::size_t>(Prime - 1, kMaxLength))
    {
        throw std::length_error("count_permutations_by_comparisons: length exceeds the limit");
    }
    if (comparisons.find_first_not_of("<>?") != std::string_view::npos)
    {
        throw std::invalid_argument("count_permutations_by_comparisons: invalid comparison");
    }
    return detail::ComparisonPermutationCounter<Prime>(comparisons.size() + 1).Count(comparisons);
}

#endif
