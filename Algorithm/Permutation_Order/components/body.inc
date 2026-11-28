#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_PERMUTATION_ORDER_DEFINED
#define SIRKIT_PERMUTATION_ORDER_DEFINED

namespace detail
{

inline fenwick_tree<int> AvailablePermutationValues(std::size_t size)
{
    CheckedFenwickStorageSize(size, std::vector<unsigned>{}.max_size());
    return fenwick_tree<int>(std::vector<int>(size, 1));
}

}

inline std::vector<int> permutation_lehmer_code(std::span<const int> values)
{
    auto unused = detail::AvailablePermutationValues(values.size());
    const int n = unused.size();
    std::vector<int> code(n);
    for (int i = 0; i < n; ++i)
    {
        const int value = values[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (value < 0 || value >= n || unused.get(value) == 0)
        {
            throw std::invalid_argument("permutation_lehmer_code: expected a permutation of 0..n-1");
        }
#endif
        code[i] = unused.sum(0, value);
        unused.add(value, -1);
    }
    return code;
}

inline std::vector<int> permutation_from_lehmer_code(std::span<const int> code)
{
    auto unused = detail::AvailablePermutationValues(code.size());
    const int n = unused.size();
    std::vector<int> values(n);
    for (int i = 0; i < n; ++i)
    {
        const int digit = code[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (digit < 0 || digit >= n - i)
        {
            throw std::invalid_argument("permutation_from_lehmer_code: digit out of range");
        }
#endif
        const int value = unused.lower_bound(digit + 1);
        values[i] = value;
        unused.add(value, -1);
    }
    return values;
}

inline std::optional<std::uint64_t> permutation_rank(std::span<const int> values)
{
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const auto code = permutation_lehmer_code(values);
    std::uint64_t rank = 0;
    for (std::size_t i = 0; i < code.size(); ++i)
    {
        const std::uint64_t radix = code.size() - i;
        const auto digit = static_cast<std::uint64_t>(code[i]);
        if (rank > (kMax - digit) / radix)
        {
            return std::nullopt;
        }
        rank = rank * radix + digit;
    }
    return rank;
}

inline std::optional<std::vector<int>> permutation_unrank(int n, std::uint64_t rank)
{
    if (n < 0)
    {
        throw std::invalid_argument("permutation_unrank: negative length");
    }
    std::vector<int> code(n);
    for (int i = n; i > 0; --i)
    {
        const auto radix = static_cast<std::uint64_t>(n - i + 1);
        code[i - 1] = static_cast<int>(rank % radix);
        rank /= radix;
    }
    if (rank != 0)
    {
        return std::nullopt;
    }
    return permutation_from_lehmer_code(code);
}

#endif
