// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TREE_DISTANCE_FREQUENCY_DEFINED
#include "../../../Graph/Centroid_Decomposition.cpp"
#include "../../../Math/NTT.cpp"
#include "../../../Graph/Tree_Distance_Frequency.cpp"
#endif

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using sirkit::Graph;
using Frequency = std::vector<std::int64_t>;

void add_edge(Graph &graph, int a, int b)
{
    graph[a].push_back(b);
    graph[b].push_back(a);
}

Frequency reference(const Graph &graph)
{
    const auto n = graph.size();
    Frequency answer(n);
    for (std::size_t start = 0; start < n; ++start)
    {
        std::vector<int> distance(n, -1);
        std::vector<std::size_t> queue{start};
        distance[start] = 0;
        for (std::size_t i = 0; i < queue.size(); ++i)
        {
            const auto v = queue[i];
            if (start < v)
            {
                ++answer[distance[v]];
            }
            for (int to : graph[v])
            {
                if (distance[to] < 0)
                {
                    distance[to] = distance[v] + 1;
                    queue.push_back(static_cast<std::size_t>(to));
                }
            }
        }
    }
    return answer;
}

void check_path_star(int path, int leaves)
{
    Graph graph(path + leaves);
    for (int v = 1; v < path; ++v)
    {
        add_edge(graph, v - 1, v);
    }
    for (int v = path; v < path + leaves; ++v)
    {
        add_edge(graph, 0, v);
    }
    const auto actual = sirkit::tree_distance_frequency(graph);
    assert(actual.size() == graph.size() && actual[0] == 0);
    for (int d = 1; d < path + leaves; ++d)
    {
        std::int64_t expected = std::max(path - d, 0);
        if (d <= path)
        {
            expected += leaves;
        }
        if (d == 2)
        {
            expected += std::int64_t{leaves} * (leaves - 1) / 2;
        }
        assert(actual[d] == expected);
    }
}

void check_two_brooms()
{
    constexpr int kHandle = 33;
    constexpr int kLeaves = 35000;
    constexpr int kOneSide = kHandle + kLeaves;
    Graph graph(1 + 2 * kOneSide);
    for (int start : {1, 1 + kOneSide})
    {
        add_edge(graph, 0, start);
        for (int i = 1; i < kHandle; ++i)
        {
            add_edge(graph, start + i - 1, start + i);
        }
        for (int v = start + kHandle; v < start + kOneSide; ++v)
        {
            add_edge(graph, start + kHandle - 1, v);
        }
    }

    Frequency expected(graph.size());
    expected[2] = std::int64_t{kLeaves} * (kLeaves - 1);
    std::vector<int> multiplicity(2 * (kHandle + 1) + 1, 1);
    multiplicity.front() = kLeaves;
    multiplicity.back() = kLeaves;
    for (std::size_t a = 0; a < multiplicity.size(); ++a)
    {
        for (std::size_t b = a + 1; b < multiplicity.size(); ++b)
        {
            expected[b - a] += std::int64_t{multiplicity[a]} * multiplicity[b];
        }
    }
    assert(expected[2 * (kHandle + 1)] > 998244353);
    assert(sirkit::tree_distance_frequency(graph) == expected);
}

template <class Exception>
void expect_error(const auto &operation)
{
    try
    {
        operation();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    using sirkit::tree_distance_frequency;
    assert(tree_distance_frequency({}).empty());
    assert(tree_distance_frequency(Graph(1)) == Frequency{0});
    check_path_star(1, 1);
    check_path_star(25000, 0);
    check_path_star(1, 60000);
    check_path_star(35000, 41000);
    check_two_brooms();

    std::mt19937 random(2975337);
    for (int n = 2; n <= 48; ++n)
    {
        for (int trial = 0; trial < 30; ++trial)
        {
            Graph graph(n);
            std::vector<int> perm(n);
            std::iota(perm.begin(), perm.end(), 0);
            std::shuffle(perm.begin(), perm.end(), random);
            for (int v = 1; v < n; ++v)
            {
                add_edge(graph, perm[v], perm[random() % v]);
            }
            assert(tree_distance_frequency(graph) == reference(graph));
        }
    }

    expect_error<std::length_error>([]
                                    {
                                        sirkit::detail::check_tree_distance_frequency_size(sirkit::detail::TreeDistanceNtt::max_length() + 1);
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &graph : {Graph{{0}}, Graph{{1}, {}}, Graph{{2}, {}},
                              Graph{{1, 1}, {0, 0}}, Graph(2), Graph{{1, 2}, {0, 2}, {0, 1}}})
    {
        expect_error<std::invalid_argument>([&]
                                            {
                                                static_cast<void>(tree_distance_frequency(graph));
                                            });
    }
#endif
}
