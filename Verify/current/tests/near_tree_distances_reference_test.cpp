// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Near_Tree_Distances.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &edges)
{
    const sirkit::NearTreeDistances distances(n, edges);
    assert(distances.size() == n);
    constexpr int kDisconnected = 1000000;
    std::vector<std::vector<int>> expected(n, std::vector<int>(n, kDisconnected));
    for (const auto &[u, v] : edges)
    {
        expected[u][v] = expected[v][u] = 1;
    }
    for (int i = 0; i < n; ++i)
    {
        expected[i][i] = 0;
    }
    for (int via = 0; via < n; ++via)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                expected[u][v] = std::min(expected[u][v], expected[u][via] + expected[via][v]);
            }
        }
    }
    for (int a = 0; a < n; ++a)
    {
        for (int b = 0; b < n; ++b)
        {
            assert(distances.distance(a, b) == (expected[a][b] == kDisconnected ? -1 : expected[a][b]));
        }
    }
}

template <class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_boundaries()
{
    check(0, {});
    check(1, {});
    check(1, {{0, 0}, {0, 0}});
    check(2, {{0, 1}, {1, 0}, {0, 1}});
    check(6, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}});
    check(12, {{0, 9}, {9, 2}, {9, 7}, {5, 7}, {2, 3}, {4, 7}, {5, 4}, {2, 4}, {6, 11}, {6, 10}, {10, 11}, {6, 8}, {8, 11}, {6, 10}});
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(sirkit::NearTreeDistances(-1, {}));
                                  });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int invalid : {-1, 2})
    {
        const std::array first{Edge{invalid, 0}};
        const std::array second{Edge{0, invalid}};
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::NearTreeDistances(2, first));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::NearTreeDistances(2, second));
                                      });
        const sirkit::NearTreeDistances distance(2, {});
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(distance.distance(invalid, 0));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(distance.distance(0, invalid));
                                      });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(110);
    for (int t = 0; t < 800; ++t)
    {
        const int n = 1 + static_cast<int>(rng() % 17);
        const unsigned m = rng() % (5 * n + 1);
        std::vector<Edge> edges;
        for (unsigned i = 0; i < m; ++i)
        {
            edges.emplace_back(rng() % n, rng() % n);
        }
        check(n, edges);
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::NearTreeDistances(-1, {}));
        return 1;
    }
    check_boundaries();
    check_random();
}
