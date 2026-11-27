// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Complement_Components.cpp"
#include <cassert>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

int main()
{
    std::mt19937 random(191);
    for (int n = 0; n <= 25; ++n)
    {
        for (int rate = 0; rate <= 16; ++rate)
        {
            std::vector<std::pair<int, int>> edges;
            std::vector<std::vector<bool>> present(n, std::vector<bool>(n));
            for (int u = 0; u < n; ++u)
            {
                for (int v = u; v < n; ++v)
                {
                    if (random() % 16 < static_cast<unsigned>(rate))
                    {
                        edges.emplace_back(u, v);
                        present[u][v] = present[v][u] = true;
                    }
                }
            }
            if (!edges.empty())
            {
                edges.push_back(edges.front());
            }
            std::vector<int> expected(n);
            std::iota(expected.begin(), expected.end(), 0);
            for (int u = 0; u < n; ++u)
            {
                for (int v = u + 1; v < n; ++v)
                {
                    if (!present[u][v])
                    {
                        const int old = expected[v], replacement = expected[u];
                        for (auto &label : expected)
                        {
                            if (label == old)
                            {
                                label = replacement;
                            }
                        }
                    }
                }
            }
            const auto result = sirkit::complement_components(n, edges);
            std::vector<int> found(n, -1);
            for (std::size_t i = 0; i < result.size(); ++i)
            {
                for (int v : result[i])
                {
                    assert(v >= 0 && v < n && found[v] == -1);
                    found[v] = static_cast<int>(i);
                }
            }
            for (int u = 0; u < n; ++u)
            {
                assert(found[u] >= 0);
                for (int v = 0; v < n; ++v)
                {
                    assert((found[u] == found[v]) == (expected[u] == expected[v]));
                }
            }
        }
    }
    try
    {
        sirkit::complement_components(-1, {});
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &bad : {std::pair{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        try
        {
            sirkit::complement_components(2, {&bad, 1});
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    try
    {
        const std::pair<int, int> edge{0, 1};
        sirkit::complement_components(1, {&edge, 1});
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
#endif
}
