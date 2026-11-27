// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Decomposition_Width_Two.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

struct BranchSet
{
    unsigned vertices;
    unsigned adjacent;
};

bool four_branch_sets(const std::vector<BranchSet> &choices, std::array<unsigned, 4> &chosen,
                      int count, std::size_t next, unsigned used)
{
    if (count == 4)
    {
        return true;
    }
    for (std::size_t i = next; i < choices.size(); ++i)
    {
        const auto [vertices, adjacent] = choices[i];
        if (vertices & used)
        {
            continue;
        }
        if (std::any_of(chosen.begin(), chosen.begin() + count,
                        [&](unsigned mask)
                        {
                            return (adjacent & mask) == 0;
                        }))
        {
            continue;
        }
        chosen[count] = vertices;
        if (four_branch_sets(choices, chosen, count + 1, i + 1, used | vertices))
        {
            return true;
        }
    }
    return false;
}

bool has_k4_minor(int n, const std::vector<Edge> &edges)
{
    std::vector<unsigned> adjacency(n);
    for (const auto &[u, v] : edges)
    {
        adjacency[u] |= 1U << v;
        adjacency[v] |= 1U << u;
    }
    std::vector<BranchSet> choices;
    for (unsigned mask = 1; mask < (1U << n); ++mask)
    {
        unsigned reached = 1U << std::countr_zero(mask);
        unsigned adjacent = 0;
        for (unsigned front = reached; front != 0;)
        {
            const auto v = std::countr_zero(front);
            front &= front - 1;
            adjacent |= adjacency[v];
            const unsigned added = adjacency[v] & mask & ~reached;
            reached |= added;
            front |= added;
        }
        if (reached == mask)
        {
            choices.push_back({mask, adjacent});
        }
    }
    // 木幅が3以上であることを、互いに接する4個の連結な頂点集合で独立に判定する。
    std::array<unsigned, 4> chosen{};
    return four_branch_sets(choices, chosen, 0, 0, 0);
}

void check_certificate(int n, const std::vector<Edge> &edges, const sirkit::WidthTwoTreeDecomposition &result)
{
    const int bags = static_cast<int>(result.bags.size());
    assert(bags == n);
    assert(result.tree_edges.size() == static_cast<std::size_t>(std::max(0, bags - 1)));

    std::vector<int> occurrences(n);
    std::set<Edge> covered;
    int width = 0;
    for (const auto &bag : result.bags)
    {
        assert(1 <= bag.size && bag.size <= 3);
        width = std::max(width, bag.size - 1);
        for (int i = 0; i < bag.size; ++i)
        {
            const int u = bag.vertices[i];
            assert(0 <= u && u < n);
            ++occurrences[u];
            for (int j = 0; j < i; ++j)
            {
                assert(u != bag.vertices[j]);
                covered.insert(std::minmax(u, bag.vertices[j]));
            }
        }
    }
    assert(result.width == width);
    for (const auto &[u, v] : edges)
    {
        assert(u == v || covered.contains(std::minmax(u, v)));
    }

    std::vector<std::vector<int>> tree(bags);
    std::vector<int> occurrences_on_edges(n);
    for (const auto &[u, v] : result.tree_edges)
    {
        assert(0 <= u && u < bags && 0 <= v && v < bags && u != v);
        tree[u].push_back(v);
        tree[v].push_back(u);
        const auto &first = result.bags[u];
        const auto &second = result.bags[v];
        for (int i = 0; i < first.size; ++i)
        {
            for (int j = 0; j < second.size; ++j)
            {
                if (first.vertices[i] == second.vertices[j])
                {
                    ++occurrences_on_edges[first.vertices[i]];
                }
            }
        }
    }
    if (bags > 0)
    {
        std::vector<bool> seen(bags);
        std::vector<int> queue{0};
        seen[0] = true;
        for (std::size_t i = 0; i < queue.size(); ++i)
        {
            for (int j : tree[queue[i]])
            {
                if (!seen[j])
                {
                    seen[j] = true;
                    queue.push_back(j);
                }
            }
        }
        assert(queue.size() == static_cast<std::size_t>(bags));
    }
    // 基本の木は連結。頂点ごとの誘導部分グラフは、辺数が袋数-1なら連結。
    for (int v = 0; v < n; ++v)
    {
        assert(occurrences[v] > 0 && occurrences_on_edges[v] + 1 == occurrences[v]);
    }
}

void check(int n, const std::vector<Edge> &edges)
{
    const auto result = sirkit::tree_decomposition_width_two(n, edges);
    assert(result.has_value() == !has_k4_minor(n, edges));
    if (result)
    {
        check_certificate(n, edges, *result);
    }
}
} // namespace

int main()
{
    using sirkit::tree_decomposition_width_two;
    check(0, {});
    check(1, {{0, 0}, {0, 0}});
    check(7, {{0, 1}, {1, 0}, {1, 2}, {2, 0}, {3, 4}, {4, 5}, {5, 6}});
    check(7, {{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}, {0, 4}, {4, 3}, {5, 6}});

    for (int n = 2; n <= 5; ++n)
    {
        std::vector<Edge> possible;
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                possible.emplace_back(i, j);
            }
        }
        const unsigned end = 1U << possible.size();
        for (unsigned mask = 0; mask < end; ++mask)
        {
            std::vector<Edge> edges;
            for (auto left = mask; left != 0; left &= left - 1)
            {
                edges.push_back(possible[std::countr_zero(left)]);
            }
            check(n, edges);
        }
    }
    std::mt19937 random(200041);
    for (int trial = 0; trial < 420; ++trial)
    {
        const int n = 6 + static_cast<int>(random() % 3);
        const unsigned threshold = random() % 11;
        std::vector<Edge> edges;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u; v < n; ++v)
            {
                if (random() % 11 < threshold)
                {
                    edges.emplace_back(u, v);
                }
                if (random() % 11 < threshold / 3)
                {
                    edges.emplace_back(v, u);
                }
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        check(n, edges);
    }

    constexpr int kLength = 4'000;
    std::vector<Edge> triangles{{0, 1}};
    for (int i = 2; i < kLength; ++i)
    {
        triangles.emplace_back(i, i - 1);
        triangles.emplace_back(i, i - 2);
    }
    const auto series = tree_decomposition_width_two(kLength, triangles);
    assert(series && series->width == 2);
    check_certificate(kLength, triangles, *series);

#ifndef SKIP_RUNTIME_CHECKS
    try
    {
        static_cast<void>(tree_decomposition_width_two(-1, {}));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    for (const Edge &invalid : std::array{Edge{-1, 0}, Edge{1, 0}, Edge{0, -1}, Edge{0, 1}})
    {
        try
        {
            static_cast<void>(tree_decomposition_width_two(1, {&invalid, 1}));
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
