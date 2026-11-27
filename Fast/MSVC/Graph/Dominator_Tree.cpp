// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Dominator_Tree.cpp
// 参考: https://ei1333.github.io/library/graph/others/dominator-tree.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dominator_tree.md
//
// API一覧
// dominator_tree(graph,root): 有向グラフの各頂点の直近支配頂点を返す。
// root自身はroot、rootから到達不能なら-1。自己ループ・多重辺も可。
// O((N+M) log(N+1))時間・O(N+M)領域。DFSと祖先の経路圧縮は反復で実行。
// rootは0以上N未満。検査有効時はrootと到達可能な頂点の出辺の行き先を検査する。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DOMINATOR_TREE_DEFINED
#define SIRKIT_DOMINATOR_TREE_DEFINED

namespace detail
{
inline void CheckDominatorTreeSize(std::size_t size)
{
    if (size >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("dominator_tree: vertex count");
    }
}
} // namespace detail

inline std::vector<int> dominator_tree(const std::vector<std::vector<int>> &graph, int root)
{
    detail::CheckDominatorTreeSize(graph.size());
    // DFS順は1始まりとし、0は未訪問および未接続を表す。
    std::vector<int> number(graph.size());
    std::vector<int> vertex{0, root}, parent{0, 0};
    struct Frame
    {
        int vertex;
        std::size_t next = 0;
    };
    std::vector<Frame> stack{{root}};
    number[root] = 1;
    while (!stack.empty())
    {
        auto &frame = stack.back();
        const int from = frame.vertex;
        if (frame.next == graph[from].size())
        {
            stack.pop_back();
            continue;
        }
        const int to = graph[from][frame.next++];
        if (number[to] == 0)
        {
            number[to] = static_cast<int>(vertex.size());
            vertex.push_back(to);
            parent.push_back(number[from]);
            stack.push_back({to});
        }
    }

    const int size = static_cast<int>(vertex.size());
    std::vector<std::vector<int>> predecessors(size), bucket(size);
    for (int i = 1; i < size; ++i)
    {
        for (int to : graph[vertex[i]])
        {
            predecessors[number[to]].push_back(i);
        }
    }

    std::vector<int> semi(size), label(size), ancestor(size), immediate(size);
    std::iota(semi.begin(), semi.end(), 0);
    std::iota(label.begin(), label.end(), 0);
    std::vector<int> path;
    const auto eval = [&](int v)
    {
        path.clear();
        for (int u = v; ancestor[ancestor[u]] != 0; u = ancestor[u])
        {
            path.push_back(u);
        }
        // 最小の半支配頂点を与えるラベルを伝えながら祖先を圧縮する。
        for (auto it = path.rbegin(); it != path.rend(); ++it)
        {
            const int u = *it;
            const int previous = ancestor[u];
            if (semi[label[previous]] < semi[label[u]])
            {
                label[u] = label[previous];
            }
            ancestor[u] = ancestor[previous];
        }
        return label[v];
    };

    for (int i = size - 1; i >= 2; --i)
    {
        for (int previous : predecessors[i])
        {
            semi[i] = std::min(semi[i], semi[eval(previous)]);
        }
        bucket[semi[i]].push_back(i);
        ancestor[i] = parent[i];
        auto &pending = bucket[parent[i]];
        for (int node : pending)
        {
            const int representative = eval(node);
            immediate[node] = semi[representative] < semi[node] ? representative : parent[i];
        }
        pending.clear();
    }

    std::vector<int> result(graph.size(), -1);
    result[root] = root;
    for (int i = 2; i < size; ++i)
    {
        if (immediate[i] != semi[i])
        {
            immediate[i] = immediate[immediate[i]];
        }
        result[vertex[i]] = vertex[immediate[i]];
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
