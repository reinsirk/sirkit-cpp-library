// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Component_Add_Union_Find.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/component_add_union_find.md
//
// API一覧
// ComponentAddUnionFind<T=int64_t>(n=0) / (span<const T> values): 各頂点を値0/指定値の単独成分にする。O(n)時間・領域。
// unite(u,v): 成分を結合。すでに同成分ならfalse。結合で頂点の値は変わらない。
// root(v) / issame(u,v) / size(v): 代表頂点/同成分か/成分の頂点数。size() は全頂点数 O(1)。
// add_component(v,delta): vの成分の全頂点に加算。get(v) / component_sum(v): 頂点の値/成分の値の合計。
//   頂点は0始まり。構築とsize()以外は償却O(α(n))。T はboolを除く64ビット以下の整数型で、型幅で周回する。
//   代表のoffsetは代表自身の値、ほかのoffsetは親との差。結合しても過去の加算は新しい頂点へ移らない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COMPONENT_ADD_UNION_FIND_DEFINED
#define SIRKIT_COMPONENT_ADD_UNION_FIND_DEFINED

namespace detail
{
inline std::size_t checked_component_add_uf_size(std::size_t n)
{
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("ComponentAddUnionFind: too many vertices");
    }
    return n;
}
}

template <std::integral T = std::int64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class ComponentAddUnionFind
{
public:
    explicit ComponentAddUnionFind(int n = 0)
    {
        if (n < 0)
        {
            throw std::length_error("ComponentAddUnionFind: negative vertex count");
        }
        nodes_.resize(n);
    }

    explicit ComponentAddUnionFind(std::span<const T> values)
        : nodes_(detail::checked_component_add_uf_size(values.size()))
    {
        for (std::size_t v = 0; v < values.size(); ++v)
        {
            const auto value = static_cast<std::uint64_t>(values[v]);
            nodes_[v].offset = value;
            nodes_[v].sum = value;
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(nodes_.size());
    }

    [[nodiscard]] int size(int v)
    {
        return -nodes_[root(v)].parent_or_size;
    }

    [[nodiscard]] int root(int v)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(v) >= nodes_.size())
        {
            throw std::out_of_range("ComponentAddUnionFind: invalid vertex");
        }
#endif
        return find_root(v);
    }

    [[nodiscard]] bool issame(int u, int v)
    {
        return root(u) == root(v);
    }

    bool unite(int u, int v)
    {
        u = root(u);
        v = root(v);
        if (u == v)
        {
            return false;
        }
        if (nodes_[u].parent_or_size > nodes_[v].parent_or_size)
        {
            std::swap(u, v);
        }
        nodes_[u].parent_or_size += nodes_[v].parent_or_size;
        nodes_[v].parent_or_size = u;
        nodes_[v].offset -= nodes_[u].offset;
        nodes_[u].sum += nodes_[v].sum;
        return true;
    }

    void add_component(int v, T delta)
    {
        auto &node = nodes_[root(v)];
        const auto bits = static_cast<std::uint64_t>(delta);
        node.offset += bits;
        node.sum += bits * static_cast<std::uint64_t>(-node.parent_or_size);
    }

    [[nodiscard]] T get(int v)
    {
        const int leader = root(v);
        const auto value = v == leader ? nodes_[v].offset : nodes_[v].offset + nodes_[leader].offset;
        return static_cast<T>(value);
    }

    [[nodiscard]] T component_sum(int v)
    {
        return static_cast<T>(nodes_[root(v)].sum);
    }

private:
    struct Node
    {
        int parent_or_size = -1;
        std::uint64_t offset = 0;
        std::uint64_t sum = 0;
    };

    int find_root(int v) noexcept
    {
        auto &node = nodes_[v];
        const int parent = node.parent_or_size;
        if (parent < 0)
        {
            return v;
        }
        const int leader = find_root(parent);
        if (leader != parent)
        {
            node.offset += nodes_[parent].offset;
            node.parent_or_size = leader;
        }
        return leader;
    }

    std::vector<Node> nodes_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
