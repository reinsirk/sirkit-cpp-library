// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Enumerate_Cliques.cpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Visit = std::function<void(std::span<const int>)>;

void check_all_subsets(int n, const std::vector<std::pair<int, int>> &edges)
{
    const unsigned subset_count = 1U << n;
    std::vector<unsigned> neighbor(n);
    for (const auto &[u, v] : edges)
    {
        neighbor[u] |= 1U << v;
        neighbor[v] |= 1U << u;
    }
    std::vector<bool> found(subset_count);
    Visit visit = [&](std::span<const int> clique)
    {
        unsigned subset = 0;
        for (int v : clique)
        {
            assert(v >= 0 && v < n);
            const auto bit = 1U << v;
            assert((subset & bit) == 0);
            subset |= bit;
        }
        assert(subset != 0 && !found[subset]);
        found[subset] = true;
    };
    sirkit::enumerate_cliques(n, edges, visit);
    for (unsigned subset = 1; subset < subset_count; ++subset)
    {
        bool expected = true;
        for (int v = 0; v < n; ++v)
        {
            const auto bit = 1U << v;
            if ((subset & bit) != 0 && ((neighbor[v] | bit) & subset) != subset)
            {
                expected = false;
                break;
            }
        }
        assert(found[subset] == expected);
    }
}

template <class Exception>
void expect_error(int n, std::initializer_list<std::pair<int, int>> edges)
{
    try
    {
        Visit visit = [](std::span<const int>)
        {
        };
        sirkit::enumerate_cliques(n, {edges.begin(), edges.size()}, visit);
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    check_all_subsets(0, {});
    std::mt19937 random(92013);
    for (int n = 1; n <= 11; ++n)
    {
        for (unsigned trial = 0; trial <= 100; ++trial)
        {
            std::vector<std::pair<int, int>> edges;
            for (int u = 0; u < n; ++u)
            {
                for (int v = u + 1; v < n; ++v)
                {
                    if (random() % 100 < trial)
                    {
                        if (random() % 2 == 0)
                        {
                            edges.emplace_back(u, v);
                        }
                        else
                        {
                            edges.emplace_back(v, u);
                        }
                    }
                }
            }
            std::shuffle(edges.begin(), edges.end(), random);
            check_all_subsets(n, edges);
        }
    }

    std::vector<std::pair<int, int>> complete;
    for (int u = 0; u < 14; ++u)
    {
        for (int v = u + 1; v < 14; ++v)
        {
            complete.emplace_back(u, v);
        }
    }
    check_all_subsets(14, complete);

    constexpr int kStarSize = 25000;
    std::vector<std::pair<int, int>> star;
    for (int v = 1; v < kStarSize; ++v)
    {
        star.emplace_back(v, 0);
    }
    std::size_t count = 0;
    Visit visit = [&](std::span<const int> clique)
    {
        assert(clique.size() == 1 || clique.size() == 2);
        ++count;
    };
    sirkit::enumerate_cliques(kStarSize, star, visit);
    assert(count == 2 * kStarSize - 1);

    expect_error<std::length_error>(-1, {});
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>(3, {{-1, 0}});
    expect_error<std::out_of_range>(3, {{3, 0}});
    expect_error<std::out_of_range>(3, {{0, -1}});
    expect_error<std::out_of_range>(3, {{0, 3}});
    expect_error<std::invalid_argument>(3, {{0, 0}});
    expect_error<std::invalid_argument>(2, {{0, 1}, {1, 0}});
    expect_error<std::invalid_argument>(4, {{0, 1}, {1, 0}});
#endif
}
