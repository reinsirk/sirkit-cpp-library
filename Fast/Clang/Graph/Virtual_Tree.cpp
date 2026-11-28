// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Virtual_Tree.cpp
// 参考（Auxiliary Treeの性質）: https://atcoder.jp/contests/abc340/editorial/9249
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/virtual_tree.md
//
// API一覧
// VirtualTree(n, edges, root=0): 頂点[0,n)の無向木を前処理する。O(n)時間・領域。n=0も可。
// build(terminals): 指定した頂点とそれらのLCAだけの木を、親が先の順に返す。入力の順序・重複は自由。
// Node{vertex, parent, distance, terminal}: 元の頂点番号・返り値中の親番号・親への辺数距離・指定頂点か。
// 根ではparent=-1、distance=0。指定が空なら空。指定数kに対しO(k log(k+1)+k log(n+1))時間・O(k)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_VIRTUAL_TREE_DEFINED
#define SIRKIT_VIRTUAL_TREE_DEFINED

class VirtualTree
{
public:
    struct Node
    {
        int vertex;
        int parent;
        int distance;
        bool terminal;
    };

    VirtualTree(int n, std::span<const std::pair<int, int>> edges, int root = 0)
    {
        if (n < 0)
        {
            throw std::length_error("VirtualTree: negative vertex count");
        }
        if (n == 0)
        {
            return;
        }

        const auto size = static_cast<std::size_t>(n);
        std::vector<std::size_t> begin(size + 1);
        for (const auto &[u, v] : edges)
        {
            ++begin[static_cast<std::size_t>(u) + 1];
            ++begin[static_cast<std::size_t>(v) + 1];
        }
        // edges.size() == n-1 なので2(n-1)は32ビットsize_tにも収まる。
        for (std::size_t i = 0; i < size; ++i)
        {
            begin[i + 1] += begin[i];
        }
        std::vector<int> adjacent(begin.back());
        auto next = begin;
        for (const auto &[u, v] : edges)
        {
            adjacent[next[u]++] = v;
            adjacent[next[v]++] = u;
        }

        parent_.resize(size);
        depth_.resize(size);
        order_.resize(size);
        subtree_size_.assign(size, 1);
        head_.resize(size);
        parent_[root] = root;
        std::vector<int> stack{root}, preorder;
        preorder.reserve(size);
        while (!stack.empty())
        {
            const int v = stack.back();
            stack.pop_back();
            order_[v] = static_cast<int>(preorder.size());
            preorder.push_back(v);
            for (std::size_t i = begin[v]; i != begin[static_cast<std::size_t>(v) + 1]; ++i)
            {
                const int to = adjacent[i];
                if (to == parent_[v])
                {
                    continue;
                }
                parent_[to] = v;
                depth_[to] = depth_[v] + 1;
                stack.push_back(to);
            }
        }

        std::vector<int> heavy_child(size, -1);
        for (std::size_t i = size; i > 1; --i)
        {
            const int v = preorder[i - 1], p = parent_[v];
            subtree_size_[p] += subtree_size_[v];
            if (heavy_child[p] == -1 || subtree_size_[heavy_child[p]] < subtree_size_[v])
            {
                heavy_child[p] = v;
            }
        }
        head_[root] = root;
        for (std::size_t i = 1; i < size; ++i)
        {
            const int v = preorder[i], p = parent_[v];
            head_[v] = v == heavy_child[p] ? head_[p] : v;
        }
    }

    [[nodiscard]] std::vector<Node> build(std::span<const int> terminals) const
    {
        if (terminals.empty())
        {
            return {};
        }
        std::vector<int> selected(terminals.begin(), terminals.end());
        const auto before = [this](int a, int b) { return order_[a] < order_[b]; };
        const auto sort_unique = [&before](std::vector<int> &vertices) {
            std::sort(vertices.begin(), vertices.end(), before);
            vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());
        };
        sort_unique(selected);
        std::vector<int> vertices = selected;
        for (std::size_t i = 1; i < selected.size(); ++i)
        {
            vertices.push_back(lca(selected[i - 1], selected[i]));
        }
        sort_unique(vertices);

        std::vector<Node> result;
        result.reserve(vertices.size());
        std::size_t terminal_index = 0;
        for (const int v : vertices)
        {
            // 追加したLCAは対応する指定頂点より前にある。最後まで比較先が残る。
            const bool terminal = selected[terminal_index] == v;
            terminal_index += terminal;
            if (result.empty())
            {
                result.push_back({v, -1, 0, terminal});
                continue;
            }
            // DFS順で直前の頂点から戻る。各頂点を親方向に通過するのは高々1回。
            int parent_index = static_cast<int>(result.size()) - 1;
            int p = result[parent_index].vertex;
            while (order_[v] >= order_[p] + subtree_size_[p])
            {
                parent_index = result[parent_index].parent;
                p = result[parent_index].vertex;
            }
            result.push_back({v, parent_index, depth_[v] - depth_[p], terminal});
        }
        return result;
    }

private:
    [[nodiscard]] int lca(int a, int b) const
    {
        while (head_[a] != head_[b])
        {
            if (depth_[head_[a]] < depth_[head_[b]])
            {
                std::swap(a, b);
            }
            a = parent_[head_[a]];
        }
        return depth_[a] < depth_[b] ? a : b;
    }

    std::vector<int> parent_, depth_, order_, subtree_size_, head_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
