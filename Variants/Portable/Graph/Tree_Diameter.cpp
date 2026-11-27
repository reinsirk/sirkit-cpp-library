// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Tree_Diameter.cpp
// 参考（2回の木の走査）: https://ei1333.github.io/luzhiled/snippets/tree/tree-diameter.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_diameter.md
//
// API一覧
// TreeDiameterEdge {from,to,weight}: 0始まりの端点と非負のstd::int64_t重みを持つ無向辺。
// TreeDiameterResult {length,path}: 直径の長さと、直径をなすパスの頂点列。
// tree_diameter(n,edges): n頂点の無向木の直径とパスを返す。O(n)時間・領域。
// n=0では{0,{}}、n=1や全辺の重みが0なら{0,{0}}。同じ長さのパスが複数あれば1つを返す。
// 長さがstd::int64_tの範囲を超えるとstd::overflow_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_DIAMETER_DEFINED
#define SIRKIT_TREE_DIAMETER_DEFINED

struct TreeDiameterEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct TreeDiameterResult
{
    std::int64_t length;
    std::vector<int> path;
};

inline TreeDiameterResult tree_diameter(int n, std::span<const TreeDiameterEdge> edges)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::invalid_argument("tree_diameter: negative vertex count");
    }
#endif
    const auto vertices = static_cast<std::size_t>(n);
#ifndef SKIP_RUNTIME_CHECKS
    if (edges.size() != (vertices == 0 ? 0 : vertices - 1))
    {
        throw std::invalid_argument("tree_diameter: wrong edge count");
    }
#endif
    if (vertices == 0)
    {
        return {0, {}};
    }
    if (vertices == 1)
    {
        return {0, {0}};
    }

    std::vector<std::size_t> offset(vertices + 1);
    for (const auto &[from, to, weight] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(from) >= vertices || static_cast<std::size_t>(to) >= vertices)
        {
            throw std::out_of_range("tree_diameter: invalid endpoint");
        }
        if (from == to || weight < 0)
        {
            throw std::invalid_argument("tree_diameter: loop or negative weight");
        }
#endif
        ++offset[from];
        ++offset[to];
    }
    std::partial_sum(offset.begin(), offset.end(), offset.begin());

    struct Arc
    {
        int to;
        std::int64_t weight;
    };
    std::vector<Arc> arcs(2 * edges.size());
    // 各頂点の区間を後ろから埋めると、offsetをそのまま区間の先頭に使える。
    for (const auto &[from, to, weight] : edges)
    {
        arcs[--offset[from]] = {to, weight};
        arcs[--offset[to]] = {from, weight};
    }

    std::vector<int> parent(vertices, -1);
    std::vector<std::int64_t> distance(vertices);
    std::vector<int> order;
    order.reserve(vertices);

    const auto farthest_from = [&](int root, [[maybe_unused]] bool check_tree)
    {
        int farthest = root;
        order.clear();
        order.push_back(root);
        parent[root] = root;
        distance[root] = 0;
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const int from = order[i];
            for (std::size_t j = offset[from]; j < offset[from + 1]; ++j)
            {
                const auto &[to, weight] = arcs[j];
                if (to == parent[from])
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (check_tree && parent[to] != -1)
                {
                    throw std::invalid_argument("tree_diameter: cycle");
                }
#endif
                if (weight > std::numeric_limits<std::int64_t>::max() - distance[from])
                {
                    throw std::overflow_error("tree_diameter: path length overflow");
                }

                parent[to] = from;
                distance[to] = distance[from] + weight;
                if (distance[to] > distance[farthest])
                {
                    farthest = to;
                }
                order.push_back(to);
            }
        }
        return farthest;
    };

    const int start = farthest_from(0, true);
#ifndef SKIP_RUNTIME_CHECKS
    if (order.size() != vertices)
    {
        throw std::invalid_argument("tree_diameter: disconnected");
    }
#endif
    const int end = farthest_from(start, false);

    TreeDiameterResult result{distance[end], {}};
    for (int vertex = end; vertex != start; vertex = parent[vertex])
    {
        result.path.push_back(vertex);
    }
    result.path.push_back(start);
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
