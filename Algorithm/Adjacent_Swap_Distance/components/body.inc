#include <functional>
#include <iterator>
#include <numeric>
#include <optional>

#ifndef SIRKIT_ADJACENT_SWAP_DISTANCE_DEFINED
#define SIRKIT_ADJACENT_SWAP_DISTANCE_DEFINED

template<class From, class To, class Compare = std::less<>>
std::optional<std::vector<std::size_t>> adjacent_swap_permutation(const From &from, const To &to, Compare compare = {})
{
    const auto n = static_cast<std::size_t>(std::size(from));
    if (n != static_cast<std::size_t>(std::size(to)))
    {
        return std::nullopt;
    }
    detail::check_range_inversion_size(n);
    const auto sorted_indices = [&](const auto &values) {
        std::vector<std::size_t> order(n);
        std::iota(order.begin(), order.end(), std::size_t{0});
        std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
            if (std::invoke(compare, values[a], values[b]))
            {
                return true;
            }
            if (std::invoke(compare, values[b], values[a]))
            {
                return false;
            }
            return a < b;
        });
        return order;
    };
    const auto sources = sorted_indices(from);
    const auto targets = sorted_indices(to);
    std::vector<std::size_t> permutation(n);
    for (std::size_t rank = 0; rank < n; ++rank)
    {
        const auto first = sources[rank];
        const auto second = targets[rank];
        if (std::invoke(compare, from[first], to[second]) || std::invoke(compare, to[second], from[first]))
        {
            return std::nullopt;
        }
        permutation[first] = second;
    }
    return permutation;
}

template<class From, class To, class Compare = std::less<>>
std::optional<std::uint64_t> adjacent_swap_distance(const From &from, const To &to, Compare compare = {})
{
    const auto permutation = adjacent_swap_permutation(from, to, std::move(compare));
    if (!permutation)
    {
        return std::nullopt;
    }
    // 同値な要素どうしの順序を保った対応なら、余分な交差がない。
    return inversion_count(*permutation);
}
#endif
