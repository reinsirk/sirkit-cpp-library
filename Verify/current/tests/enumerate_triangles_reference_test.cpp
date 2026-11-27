// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Enumerate_Triangles.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <set>
#include <utility>
#include <vector>

int main()
{
    std::mt19937 random(16449);
    std::set<std::array<int, 3>> result;
    const auto visit = [&](int u, int v, int w)
    {
        std::array triangle{u, v, w};
        std::ranges::sort(triangle);
        assert(result.insert(triangle).second);
    };
    for (int n = 0; n < 23; ++n)
    {
        for (unsigned rate = 0; rate <= 24; ++rate)
        {
            result.clear();
            std::vector<std::pair<int, int>> edges;
            std::vector<std::vector<bool>> present(n, std::vector<bool>(n));
            for (int u = 0; u < n; ++u)
            {
                for (int v = u + 1; v < n; ++v)
                {
                    if (random() % 24 < rate)
                    {
                        if (random() & 1U)
                        {
                            edges.emplace_back(u, v);
                        }
                        else
                        {
                            edges.emplace_back(v, u);
                        }
                        present[u][v] = true;
                    }
                }
            }
            sirkit::enumerate_triangles(n, edges, visit);
            for (int u = 0; u < n; ++u)
            {
                for (int v = u + 1; v < n; ++v)
                {
                    for (int w = v + 1; w < n; ++w)
                    {
                        assert(result.contains({u, v, w}) == (present[u][v] && present[v][w] && present[u][w]));
                    }
                }
            }
        }
    }
    try
    {
        sirkit::enumerate_triangles(-1, {}, visit);
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
            sirkit::enumerate_triangles(2, {&bad, 1}, visit);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    for (auto bad : {std::pair{0, 0}, std::pair{0, 2}})
    {
        try
        {
            sirkit::enumerate_triangles(2, {&bad, 1}, visit);
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
            assert(bad.second == 0);
        }
        catch (const std::out_of_range &)
        {
            assert(bad.second == 2);
        }
    }
#endif
}
