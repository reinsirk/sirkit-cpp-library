// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/equal_tree_path_partition.md
//
// API一覧
// partition_forest_into_equal_paths(n,edges,path_size): 森の頂点を同じ頂点数のパスに分ける。
//   頂点は0始まり。edges は無向辺 (u,v) の列。path_size は1以上で、各パスに含める頂点数。
//   分解できれば各パスの頂点列を返す。できなければ nullopt。時間・領域 O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EQUAL_TREE_PATH_PARTITION_DEFINED
#define SIRKIT_EQUAL_TREE_PATH_PARTITION_DEFINED

inline std::optional<std::vector<std::vector<int>>> partition_forest_into_equal_paths(
    int n, std::span<const std::pair<int, int>> edges, int path_size)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::length_error("partition_forest_into_equal_paths: negative vertex count");
    }
    if (path_size <= 0)
    {
        throw std::invalid_argument("partition_forest_into_equal_paths: path size must be positive");
    }
#endif
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("partition_forest_into_equal_paths: invalid endpoint");
        }
        if (u == v)
        {
            throw std::invalid_argument("partition_forest_into_equal_paths: self-loop");
        }
#endif
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    std::vector<int> parent(n, -1);
    std::vector<int> order;
    order.reserve(static_cast<std::size_t>(n));
    for (int root = 0; root < n; ++root)
    {
        if (parent[root] != -1)
        {
            continue;
        }
        parent[root] = root;
        auto next = order.size();
        order.push_back(root);
        while (next != order.size())
        {
            const int u = order[next++];
            for (int v : graph[u])
            {
                if (v == parent[u])
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (parent[v] != -1)
                {
                    throw std::invalid_argument("partition_forest_into_equal_paths: graph must be a forest");
                }
#endif
                parent[v] = u;
                order.push_back(v);
            }
        }
    }

    std::vector<int> remainder(n), first(n, -1), second(n, -1);
    std::vector<int> closed;
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int u = *it;
        int length = 1;
        // 子部分木から出せる未完成のパスは1本まで。2本をつないだらここで閉じる。
        for (int v : graph[u])
        {
            if (v == parent[u] || remainder[v] == 0)
            {
                continue;
            }
            if (first[u] == -1)
            {
                first[u] = v;
            }
            else if (second[u] == -1)
            {
                second[u] = v;
            }
            else
            {
                return std::nullopt;
            }
            length += remainder[v];
        }
        if (length == path_size)
        {
            closed.push_back(u);
        }
        else
        {
            if (length > path_size || second[u] != -1 || parent[u] == u)
            {
                return std::nullopt;
            }
            remainder[u] = length;
        }
    }

    std::vector<std::vector<int>> result;
    result.reserve(closed.size());
    for (int u : closed)
    {
        auto &path = result.emplace_back();
        path.reserve(static_cast<std::size_t>(path_size));
        for (int v = u; v != -1; v = first[v])
        {
            path.push_back(v);
        }
        std::ranges::reverse(path);
        for (int v = second[u]; v != -1; v = first[v])
        {
            path.push_back(v);
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
