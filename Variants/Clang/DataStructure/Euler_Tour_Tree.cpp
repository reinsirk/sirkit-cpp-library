// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Euler_Tour_Tree.cpp
// 参考（動的木のEuler Tour表現）: https://doi.org/10.1145/320211.320215
// 参考（splay木）: https://doi.org/10.1145/3828.3835
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/euler_tour_tree.md
//
// API一覧
// EulerTourTree<T>(n,initial={}): n個の孤立頂点を作る。初期値を省略するとすべて0。O(n+1)時間・領域。
// size()/component_count(): 頂点数/連結成分数。各O(1)。頂点は[0,n)、空の森も可。
// connected(u,v): 同じ成分か。link(u,v): 別成分なら辺を追加。cut(u,v): 辺があれば削除。
// link/cutは森の辺を変更したときtrue。自己辺や既に連結な頂点をlinkするとfalse。
// get(v)/add(v,delta): 点の値を読む/加算。component_sum(v)/component_add(v,delta): 成分の和/一括加算。
// subtree_sum(v,p)/subtree_add(v,p,delta): 辺(v,p)から見たv側の和/一括加算。辺がなければnullopt/false。
// 更新・照会は償却O(log(n+1))、再帰なし。照会も内部のsplayを行う。
// T{}は加法の単位元。可換な+=とT(頂点数)との乗算を用い、途中結果もTで表現できること。
// 操作中のT{}・T(頂点数)の構築、演算、代入は例外を送出しないこと。
// 3n−2をintで表せる範囲。コピーは独立でO(n+1)、移動・swapはO(1)。移動元は空の森。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EULER_TOUR_TREE_DEFINED
#define SIRKIT_EULER_TOUR_TREE_DEFINED

template <class T = std::int64_t>
class EulerTourTree
{
    struct Node
    {
        int parent = 0;
        int child[2]{};
        int count = 0;
        bool vertex = false;
        bool pending = false;
        T value{}, sum{}, lazy{};
    };

    using Edge = std::pair<int, int>;

    int vertices_ = 0;
    std::vector<Node> nodes_;
    std::map<Edge, int> darts_;
    std::vector<int> available_, ancestors_;
    std::size_t next_dart_ = 1;

    static std::size_t capacity(int n, std::size_t initial_size)
    {
        constexpr auto kMaxIndex = static_cast<std::size_t>(std::numeric_limits<int>::max());
        if (n < 0 || static_cast<std::size_t>(n) > (kMaxIndex + 2) / 3)
        {
            throw std::length_error("EulerTourTree: vertex count exceeds capacity");
        }
        if (initial_size != 0 && initial_size != static_cast<std::size_t>(n))
        {
            throw std::invalid_argument("EulerTourTree: incorrect number of initial values");
        }
        return n == 0 ? 1 : 3 * static_cast<std::size_t>(n) - 1;
    }

    void reserve_work()
    {
        available_.reserve(static_cast<std::size_t>(std::max(vertices_ - 1, 0)));
        ancestors_.reserve(nodes_.size());
    }

    void check_vertex([[maybe_unused]] int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || v >= vertices_)
        {
            throw std::out_of_range("EulerTourTree: vertex out of range");
        }
#endif
    }

    void pull(int v)
    {
        auto &node = nodes_[v];
        const auto &left = nodes_[node.child[0]];
        const auto &right = nodes_[node.child[1]];
        node.count = left.count + right.count + static_cast<int>(node.vertex);
        node.sum = left.sum;
        node.sum += node.value;
        node.sum += right.sum;
    }

    void apply(int v, const T &delta)
    {
        auto &node = nodes_[v];
        if (node.count == 0)
        {
            return;
        }
        node.sum += delta * T(node.count);
        if (node.vertex)
        {
            node.value += delta;
        }
        node.lazy += delta;
        node.pending = true;
    }

    void push(int v)
    {
        auto &node = nodes_[v];
        if (node.pending)
        {
            apply(node.child[0], node.lazy);
            apply(node.child[1], node.lazy);
            node.lazy = T{};
            node.pending = false;
        }
    }

    void rotate(int v)
    {
        const int parent = nodes_[v].parent;
        const int grand = nodes_[parent].parent;
        const int side = nodes_[parent].child[1] == v;
        const int middle = nodes_[v].child[side ^ 1];
        if (grand != 0)
        {
            nodes_[grand].child[nodes_[grand].child[1] == parent] = v;
        }
        nodes_[v].parent = grand;
        nodes_[parent].child[side] = middle;
        if (middle != 0)
        {
            nodes_[middle].parent = parent;
        }
        nodes_[v].child[side ^ 1] = parent;
        nodes_[parent].parent = v;
        pull(parent);
        pull(v);
    }

    void splay(int v)
    {
        ancestors_.clear();
        for (int at = v; at != 0; at = nodes_[at].parent)
        {
            ancestors_.push_back(at);
        }
        for (auto it = ancestors_.rbegin(); it != ancestors_.rend(); ++it)
        {
            push(*it);
        }
        while (nodes_[v].parent != 0)
        {
            const int parent = nodes_[v].parent, grand = nodes_[parent].parent;
            if (grand != 0)
            {
                if ((nodes_[grand].child[0] == parent) == (nodes_[parent].child[0] == v))
                {
                    rotate(parent);
                }
                else
                {
                    rotate(v);
                }
            }
            rotate(v);
        }
    }

    int concatenate(int left, int right)
    {
        int last = left;
        while (nodes_[last].child[1] != 0)
        {
            last = nodes_[last].child[1];
        }
        splay(last);
        nodes_[last].child[1] = right;
        nodes_[right].parent = last;
        pull(last);
        return last;
    }

    int rotate_front(int v)
    {
        splay(v);
        const int left = nodes_[v].child[0];
        if (left == 0)
        {
            return v;
        }
        nodes_[left].parent = 0;
        nodes_[v].child[0] = 0;
        pull(v);
        return concatenate(v, left);
    }

    bool connected_nodes(int a, int b)
    {
        if (a == b)
        {
            return true;
        }
        splay(a);
        splay(b);
        return nodes_[a].parent != 0;
    }

    static Edge undirected(int u, int v)
    {
        return std::minmax(u, v);
    }

    int subtree_node(int v, int parent)
    {
        const auto it = darts_.find(undirected(v, parent));
        if (it == darts_.end())
        {
            return 0;
        }
        const int return_dart = it->second + static_cast<int>(v > parent);
        // 親から入る辺を先頭にして、逆向きの辺までに出会う頂点を集める。
        rotate_front(it->second + static_cast<int>(parent > v));
        splay(return_dart);
        return nodes_[return_dart].child[0];
    }

public:
    explicit EulerTourTree(int n, std::span<const T> initial = {})
        : vertices_(n), nodes_(capacity(n, initial.size())), next_dart_(static_cast<std::size_t>(n) + 1)
    {
        reserve_work();
        for (int v = 0; v < n; ++v)
        {
            auto &node = nodes_[v + 1];
            node.vertex = true;
            node.count = 1;
            if (!initial.empty())
            {
                node.value = node.sum = initial[v];
            }
        }
    }

    EulerTourTree(const EulerTourTree &other)
        : vertices_(other.vertices_), nodes_(other.nodes_), darts_(other.darts_),
          available_(other.available_), next_dart_(other.next_dart_)
    {
        reserve_work();
    }

    EulerTourTree(EulerTourTree &&other)
    {
        swap(other);
    }

    EulerTourTree &operator=(EulerTourTree other) noexcept
    {
        swap(other);
        return *this;
    }

    void swap(EulerTourTree &other) noexcept
    {
        std::swap(vertices_, other.vertices_);
        std::swap(next_dart_, other.next_dart_);
        nodes_.swap(other.nodes_);
        darts_.swap(other.darts_);
        available_.swap(other.available_);
        ancestors_.swap(other.ancestors_);
    }

    int size() const noexcept
    {
        return vertices_;
    }

    int component_count() const noexcept
    {
        return vertices_ - static_cast<int>(darts_.size());
    }

    bool connected(int u, int v)
    {
        check_vertex(u);
        check_vertex(v);
        return connected_nodes(u + 1, v + 1);
    }

    bool link(int u, int v)
    {
        check_vertex(u);
        check_vertex(v);
        if (connected_nodes(u + 1, v + 1))
        {
            return false;
        }
        const bool new_pair = available_.empty();
        const int base = new_pair ? static_cast<int>(next_dart_) : available_.back();
        darts_.emplace(undirected(u, v), base);
        if (new_pair)
        {
            next_dart_ += 2;
        }
        else
        {
            available_.pop_back();
        }

        const int outward = base + static_cast<int>(u > v);
        const int inward = base + static_cast<int>(v > u);
        const int first = rotate_front(u + 1);
        const int second = rotate_front(v + 1);
        concatenate(concatenate(first, outward), concatenate(second, inward));
        return true;
    }

    bool cut(int u, int v)
    {
        check_vertex(u);
        check_vertex(v);
        const auto it = darts_.find(undirected(u, v));
        if (it == darts_.end())
        {
            return false;
        }
        const int outgoing = it->second, incoming = outgoing + 1;
        rotate_front(outgoing);
        splay(incoming);
        const auto &between = nodes_[incoming].child;
        const int on_path = between[0], outside = between[1];
        nodes_[on_path].parent = 0;
        nodes_[outside].parent = 0;
        // 先頭の往路、各側の巡回、復路の形になっている。往路と復路だけを外す。
        splay(outgoing);
        nodes_[nodes_[outgoing].child[1]].parent = 0;
        nodes_[outgoing] = Node{};
        nodes_[incoming] = Node{};
        available_.push_back(outgoing);
        darts_.erase(it);
        return true;
    }

    T get(int v)
    {
        check_vertex(v);
        splay(v + 1);
        return nodes_[v + 1].value;
    }

    void add(int v, const T &delta)
    {
        check_vertex(v);
        splay(v + 1);
        nodes_[v + 1].value += delta;
        pull(v + 1);
    }

    T component_sum(int v)
    {
        check_vertex(v);
        splay(v + 1);
        return nodes_[v + 1].sum;
    }

    void component_add(int v, const T &delta)
    {
        check_vertex(v);
        splay(v + 1);
        apply(v + 1, delta);
    }

    std::optional<T> subtree_sum(int v, int parent)
    {
        check_vertex(v);
        check_vertex(parent);
        const int node = subtree_node(v, parent);
        if (node == 0)
        {
            return std::nullopt;
        }
        return nodes_[node].sum;
    }

    bool subtree_add(int v, int parent, const T &delta)
    {
        check_vertex(v);
        check_vertex(parent);
        const int node = subtree_node(v, parent);
        if (node == 0)
        {
            return false;
        }
        apply(node, delta);
        // 復路マーカーが根なので、部分木を変えた後はその集約も更新する。
        pull(nodes_[node].parent);
        return true;
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
