// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Vertex_Deletion_Components.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/vertex_deletion_components.md
//
// API一覧
// vertex_deletion_components(n, edges): 0 始まりの無向グラフ。自己辺・多重辺・孤立頂点を扱う。
// VertexDeletionComponentsResult{original_count,after_deleting,separated_pairs}:
//   original_count は元の連結成分数。after_deleting[v] は v を1つ除いたときの全体の連結成分数。
//   separated_pairs[v] は v を除くことで初めて非連結になる他の2頂点の組数（順序なし）。
//   空グラフの元の成分数は0、結果の配列は空。反復DFS、O(n+m+1) 時間・領域。
//   親辺を含めても、low が親の訪問時刻以上かどうかで親を削除したときの分割を判定できる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_VERTEX_DELETION_COMPONENTS_DEFINED
#define SIRKIT_VERTEX_DELETION_COMPONENTS_DEFINED

struct VertexDeletionComponentsResult
{
    int original_count;
    std::vector<int> after_deleting;
    std::vector<std::uint64_t> separated_pairs;
};

[[nodiscard]] inline VertexDeletionComponentsResult vertex_deletion_components(
    int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("vertex_deletion_components: negative vertex count");
    }
    std::vector<std::size_t> offsets(static_cast<std::size_t>(n) + 1);
    for (const auto &[u, v] : edges)
    {
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("vertex_deletion_components: edge endpoint");
        }
        ++offsets[u + 1];
        ++offsets[v + 1];
    }
    std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
    std::vector<int> adjacent(offsets.back());
    {
        auto cursor = offsets;
        for (const auto &[u, v] : edges)
        {
            adjacent[cursor[u]++] = v;
            adjacent[cursor[v]++] = u;
        }
    }

    struct Frame
    {
        int vertex;
        std::size_t next;
    };
    std::vector<Frame> stack;
    std::vector<int> order, entry(n, -1), low(n), subtree(n, 1), split_size(n);
    stack.reserve(n);
    order.reserve(n);
    VertexDeletionComponentsResult result{0, std::vector<int>(n, 1), std::vector<std::uint64_t>(n)};
    for (int root = 0; root < n; ++root)
    {
        if (entry[root] != -1)
        {
            continue;
        }
        ++result.original_count;
        const auto component_start = order.size();
        low[root] = entry[root] = static_cast<int>(order.size());
        order.push_back(root);
        stack.push_back({root, offsets[root]});
        result.after_deleting[root] = 0;
        while (!stack.empty())
        {
            auto &frame = stack.back();
            const int v = frame.vertex;
            if (frame.next < offsets[v + 1])
            {
                const int w = adjacent[frame.next++];
                if (entry[w] == -1)
                {
                    low[w] = entry[w] = static_cast<int>(order.size());
                    order.push_back(w);
                    stack.push_back({w, offsets[w]});
                }
                else
                {
                    low[v] = std::min(low[v], entry[w]);
                }
                continue;
            }

            stack.pop_back();
            if (stack.empty())
            {
                continue;
            }
            const int parent = stack.back().vertex;
            subtree[parent] += subtree[v];
            low[parent] = std::min(low[parent], low[v]);
            if (low[v] >= entry[parent])
            {
                ++result.after_deleting[parent];
                result.separated_pairs[parent] += static_cast<std::uint64_t>(split_size[parent]) * subtree[v];
                split_size[parent] += subtree[v];
            }
        }
        for (std::size_t i = component_start; i < order.size(); ++i)
        {
            const int v = order[i];
            const int rest = subtree[root] - 1 - split_size[v];
            result.separated_pairs[v] += static_cast<std::uint64_t>(split_size[v]) * rest;
        }
    }
    for (auto &pieces : result.after_deleting)
    {
        pieces += result.original_count - 1;
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
