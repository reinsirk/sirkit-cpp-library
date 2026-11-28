// SPDX-License-Identifier: CC0-1.0
#include "Graph/Tree_Subset_Medians.cpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edges = std::vector<std::pair<int, int>>;

std::uint64_t choose(std::size_t n, std::size_t k)
{
    std::uint64_t count = 1;
    for (std::size_t i = 0; i < k; ++i)
    {
        count = count * (n - i) / (i + 1);
    }
    return count;
}

bool agrees(std::size_t n, const Edges &edges, const std::vector<std::uint8_t> &eligible)
{
    const sirkit::TreeSubsetMedians medians(n, edges, eligible);
    const auto count = eligible.empty() ? n : static_cast<std::size_t>(std::ranges::count_if(eligible, [](std::uint8_t value)
                                                                                             {
                                                                                                 return value != 0;
                                                                                             }));
    if (medians.size() != n || medians.eligible_count() != count || medians.median_count_sum(count + 1, choose) != 0)
    {
        return false;
    }
    std::vector<std::vector<int>> d(n, std::vector<int>(n, static_cast<int>(n) + 1));
    for (std::size_t i = 0; i < n; ++i)
    {
        d[i][i] = 0;
    }
    for (const auto &[u, v] : edges)
    {
        d[u][v] = d[v][u] = 1;
    }
    for (std::size_t k = 0; k < n; ++k)
    {
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < n; ++j)
            {
                d[i][j] = std::min(d[i][j], d[i][k] + d[k][j]);
            }
        }
    }
    std::vector<std::size_t> histogram(count / 2 + 1);
    for (const auto &[u, v] : edges)
    {
        std::size_t side = 0;
        for (std::size_t w = 0; w < n; ++w)
        {
            side += (eligible.empty() || eligible[w] != 0) && d[u][w] < d[v][w];
        }
        ++histogram[std::min(side, count - side)];
    }
    if (histogram != medians.cut_histogram())
    {
        return false;
    }
    std::vector<std::uint64_t> total(count + 1);
    std::uint32_t allowed = 0;
    for (std::size_t v = 0; v < n; ++v)
    {
        if (eligible.empty() || eligible[v] != 0)
        {
            allowed |= std::uint32_t{1} << v;
        }
    }
    for (std::uint32_t mask = 0; mask < (std::uint32_t{1} << n); ++mask)
    {
        if ((mask & ~allowed) != 0)
        {
            continue;
        }
        int best = std::numeric_limits<int>::max(), tied = 0;
        for (std::size_t v = 0; v < n; ++v)
        {
            int distance_sum = 0;
            for (std::size_t w = 0; w < n; ++w)
            {
                distance_sum += ((mask >> w) & 1) != 0 ? d[v][w] : 0;
            }
            if (distance_sum < best)
            {
                tied = 0;
                best = distance_sum;
            }
            tied += distance_sum == best;
        }
        total[static_cast<std::size_t>(std::popcount(mask))] += static_cast<std::uint64_t>(tied);
    }
    for (std::size_t k = 0; k <= count; ++k)
    {
        if (medians.median_count_sum(k, choose) != total[k])
        {
            return false;
        }
    }
    return true;
}
}

int main()
{
    if (!agrees(0, {}, {}) || !agrees(3, {{2, 1}, {1, 0}}, {0, 0, 0}) || !agrees(3, {{0, 2}, {1, 2}}, {2, 255, 0}))
    {
        return 1;
    }
    std::mt19937 generator(946215);
    for (int n = 1; n <= 10; ++n)
    {
        for (int test = 0; test < 15; ++test)
        {
            Edges edges;
            for (int v = 1; v < n; ++v)
            {
                edges.emplace_back(v, static_cast<int>(generator() % v));
            }
            std::ranges::shuffle(edges, generator);
            std::vector<std::uint8_t> eligible(n);
            for (auto &bit : eligible)
            {
                bit = static_cast<std::uint8_t>(generator() % 2);
            }
            if (!agrees(static_cast<std::size_t>(n), edges, eligible) || !agrees(static_cast<std::size_t>(n), edges, {}))
            {
                return 2;
            }
        }
    }
    try
    {
        static_cast<void>(sirkit::TreeSubsetMedians(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1, {}));
        return 3;
    }
    catch (const std::length_error &)
    {
    }
    for (const auto &edges : {Edges{{3, 0}}, Edges{{0, -1}}})
    {
        try
        {
            static_cast<void>(sirkit::TreeSubsetMedians(2, edges));
            return 4;
        }
        catch (const std::out_of_range &)
        {
        }
    }
    for (const auto &edges : {Edges{}, Edges{{0, 1}, {1, 0}}})
    {
        try
        {
            static_cast<void>(sirkit::TreeSubsetMedians(3, edges));
            return 5;
        }
        catch (const std::invalid_argument &)
        {
        }
    }
    try
    {
        const std::vector<std::uint8_t> wrong_size{1, 1};
        static_cast<void>(sirkit::TreeSubsetMedians(1, {}, wrong_size));
        return 6;
    }
    catch (const std::invalid_argument &)
    {
    }
}
