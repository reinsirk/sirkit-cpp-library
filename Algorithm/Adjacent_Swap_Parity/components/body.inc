#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_ADJACENT_SWAP_PARITY_DEFINED
#define SIRKIT_ADJACENT_SWAP_PARITY_DEFINED

struct AdjacentSwapParityDistances
{
    std::optional<std::uint64_t> even;
    std::optional<std::uint64_t> odd;
};

template <class From, class To, class Compare = std::less<>>
[[nodiscard]] AdjacentSwapParityDistances adjacent_swap_distances_by_parity(const From &from, const To &to, Compare compare = {})
{
    const auto n = static_cast<std::size_t>(std::size(from));
    if (n != static_cast<std::size_t>(std::size(to)))
    {
        return {};
    }
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("adjacent_swap_distances_by_parity: input too long");
    }
    const auto matching = adjacent_swap_permutation(from, to, std::ref(compare));
    if (!matching)
    {
        return {};
    }
    const auto &permutation = *matching;
    std::vector<std::size_t> sorted(n);
    std::iota(sorted.begin(), sorted.end(), std::size_t{0});
    std::ranges::sort(sorted, [&](std::size_t a, std::size_t b)
                      {
                          if (std::invoke(compare, from[a], from[b]))
                          {
                              return true;
                          }
                          if (std::invoke(compare, from[b], from[a]))
                          {
                              return false;
                          }
                          return a < b;
                      });
    std::vector<std::size_t> before(n, n), after(n, n);
    for (std::size_t i = 1; i < n; ++i)
    {
        const auto l = sorted[i - 1], r = sorted[i];
        if (!std::invoke(compare, from[l], from[r]))
        {
            before[r] = l;
            after[l] = r;
        }
    }

    fenwick_tree<std::uint32_t> seen(static_cast<int>(n));
    const auto between_values = [&](std::size_t l, std::size_t r)
    {
        return seen.sum(static_cast<int>(permutation[l] + 1), static_cast<int>(permutation[r]));
    };
    std::vector<std::uint32_t> before_left(n);
    std::uint64_t minimum = 0;
    auto additional = std::numeric_limits<std::uint64_t>::max();
    for (std::size_t i = 0; i < n; ++i)
    {
        const auto value = static_cast<int>(permutation[i]);
        minimum += i - seen.sum(0, value);
        if (before[i] != n)
        {
            const auto inside = between_values(before[i], i) - before_left[i]; // 端点を交差させると、内部の対応点ごとに2転倒増える。
            additional = std::min(additional, 2 * std::uint64_t{inside} + 1);
        }
        if (after[i] != n)
        {
            before_left[after[i]] = between_values(i, after[i]);
        }
        seen.add(value, 1);
    }

    std::optional<std::uint64_t> opposite;
    if (additional != std::numeric_limits<std::uint64_t>::max())
    {
        opposite = minimum + additional;
    }
    if ((minimum & 1u) == 0)
    {
        return {minimum, opposite};
    }
    return {opposite, minimum};
}

#endif
