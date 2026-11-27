// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Tree_Topological_Order_Minimum_Inversions.cpp
// 参考（木で制約された仕事の順序付け）: https://doi.org/10.1137/0123021
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_topological_order_minimum_inversions.md
//
// API一覧
// minimum_inversion_topological_order(n,parent,c,d): 親を先に並べ、先行するuと後続vのd[u]*c[v]の積和を最小にする。
// parent[v]は頂点vの親（0始まり）、根だけ-1。c,dは非負整数の頂点重み。各列の長さはn。
// .inversionsは最小値、.orderはその値を達成する頂点順。空グラフは積和0・順序空。
// c,dそれぞれの総和や最小値がuint64_tの範囲外ならoverflow_error。
// O(n log(n+1))時間・O(n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstdint>
#include <limits>
#include <numeric>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_TOPOLOGICAL_ORDER_MINIMUM_INVERSIONS_DEFINED
#define SIRKIT_TREE_TOPOLOGICAL_ORDER_MINIMUM_INVERSIONS_DEFINED

struct MinimumInversionTopologicalOrder
{
    std::uint64_t inversions;
    std::vector<int> order;
};

namespace detail
{
using TreeOrderWeight = std::uint64_t;
inline constexpr TreeOrderWeight kTreeOrderMaxWeight = std::numeric_limits<TreeOrderWeight>::max();

inline void tree_order_add(TreeOrderWeight &total, TreeOrderWeight value)
{
    if (value > kTreeOrderMaxWeight - total)
    {
        throw std::overflow_error("minimum_inversion_topological_order: sum does not fit uint64_t");
    }
    total += value;
}

inline std::pair<TreeOrderWeight, TreeOrderWeight> tree_order_product(TreeOrderWeight a, TreeOrderWeight b)
{
    // 32ビットずつの積を組み立て、128ビット積を上位・下位64ビットで返す。
    constexpr TreeOrderWeight kMask = (TreeOrderWeight{1} << 32) - 1;
    const auto a_low = a & kMask, a_high = a >> 32;
    const auto b_low = b & kMask, b_high = b >> 32;
    const auto low = a_low * b_low;
    const auto middle = a_high * b_low + (low >> 32);
    const auto carry = a_low * b_high + (middle & kMask);
    return {a_high * b_high + (middle >> 32) + (carry >> 32), (carry << 32) | (low & kMask)};
}

struct TreeOrderBlock
{
    TreeOrderWeight c;
    TreeOrderWeight d;
    int vertex;
    int version;

    TreeOrderWeight denominator() const
    {
        return (c | d) == 0 ? 1 : d;
    }
};

struct TreeOrderDensityLess
{
    bool wide;

    bool operator()(const TreeOrderBlock &a, const TreeOrderBlock &b) const
    {
        const auto da = a.denominator(), db = b.denominator();
        const auto lhs = wide ? tree_order_product(a.c, db) : std::pair{TreeOrderWeight{0}, a.c * db};
        const auto rhs = wide ? tree_order_product(b.c, da) : std::pair{TreeOrderWeight{0}, b.c * da};
        return lhs == rhs ? a.vertex > b.vertex : lhs < rhs;
    }
};

inline int tree_order_root(std::span<const int> parent)
{
    const auto n = static_cast<int>(parent.size());
    int root = -1;
    for (int v = 0; v < n; ++v)
    {
        const int p = parent[v];
#ifndef SKIP_RUNTIME_CHECKS
        if (p < -1 || p >= n)
        {
            throw std::out_of_range("minimum_inversion_topological_order: parent out of range");
        }
#endif
        if (p == -1)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (root != -1)
            {
                throw std::invalid_argument("minimum_inversion_topological_order: multiple roots");
            }
#endif
            root = v;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (root == -1)
    {
        throw std::invalid_argument("minimum_inversion_topological_order: missing root");
    }
    std::vector<unsigned char> seen(n);
    seen[root] = 2;
    for (int v = 0; v < n; ++v)
    {
        int u = v;
        for (; seen[u] == 0; u = parent[u])
        {
            seen[u] = 1;
        }
        if (seen[u] == 1)
        {
            throw std::invalid_argument("minimum_inversion_topological_order: cycle in parents");
        }
        for (u = v; seen[u] == 1; u = parent[u])
        {
            seen[u] = 2;
        }
    }
#endif
    return root;
}

class TreeOrderBlocks
{
    std::vector<int> link_;
    std::vector<int> top_;

    int find(int v)
    {
        while (link_[v] >= 0)
        {
            const int p = link_[v];
            if (link_[p] >= 0)
            {
                link_[v] = link_[p];
            }
            v = link_[v];
        }
        return v;
    }

public:
    explicit TreeOrderBlocks(int n) : link_(n, -1), top_(n)
    {
        std::iota(top_.begin(), top_.end(), 0);
    }

    int top(int v)
    {
        return top_[find(v)];
    }

    void append(int parent, int child)
    {
        int a = find(parent), b = find(child);
        if (link_[a] > link_[b])
        {
            std::swap(a, b);
        }
        link_[a] += link_[b];
        link_[b] = a;
        top_[a] = parent;
    }
};
} // namespace detail

inline MinimumInversionTopologicalOrder minimum_inversion_topological_order(
    int n, std::span<const int> parent, std::span<const std::uint64_t> c, std::span<const std::uint64_t> d)
{
    if (n < 0 || parent.size() != static_cast<std::size_t>(n) ||
        c.size() != parent.size() || d.size() != parent.size())
    {
        throw std::invalid_argument("minimum_inversion_topological_order: inconsistent lengths");
    }
    if (n == 0)
    {
        return {0, {}};
    }
    const int root = detail::tree_order_root(parent);
    using Weight = detail::TreeOrderWeight;
    Weight total_c = 0, total_d = 0;
    std::vector<detail::TreeOrderBlock> blocks, initial;
    blocks.reserve(n);
    initial.reserve(n - 1);
    for (int v = 0; v < n; ++v)
    {
        detail::tree_order_add(total_c, c[v]);
        detail::tree_order_add(total_d, d[v]);
        blocks.push_back({c[v], d[v], v, 0});
        if (v != root)
        {
            initial.push_back(blocks.back());
        }
    }
    const bool wide = total_c != 0 && total_d > detail::kTreeOrderMaxWeight / total_c;
    std::priority_queue<detail::TreeOrderBlock, std::vector<detail::TreeOrderBlock>, detail::TreeOrderDensityLess>
        queue(detail::TreeOrderDensityLess{wide}, std::move(initial));

    detail::TreeOrderBlocks merged(n);
    std::vector<int> tail(n), next(n, -1);
    std::iota(tail.begin(), tail.end(), 0);
    while (!queue.empty())
    {
        const auto [child_c, child_d, child, stamp] = queue.top();
        queue.pop();
        if (blocks[child].version != stamp)
        {
            continue;
        }
        // 最大c/d比の子を親ブロックの直後に置いても、途中のブロックとの交換で積和が増えない。
        const int above = merged.top(parent[child]);
        auto &block = blocks[above];
        block.c += child_c;
        block.d += child_d;
        next[tail[above]] = child;
        tail[above] = tail[child];
        merged.append(above, child);
        blocks[child].version = -1;
        if (above != root)
        {
            ++block.version;
            queue.push(block);
        }
    }

    MinimumInversionTopologicalOrder result{0, {}};
    result.order.reserve(n);
    Weight prefix = 0;
    for (int v = root; v != -1; v = next[v])
    {
        result.order.push_back(v);
        if (wide)
        {
            if (c[v] != 0 && prefix > detail::kTreeOrderMaxWeight / c[v])
            {
                throw std::overflow_error("minimum_inversion_topological_order: inversion term does not fit uint64_t");
            }
            detail::tree_order_add(result.inversions, c[v] * prefix);
        }
        else
        {
            result.inversions += c[v] * prefix;
        }
        prefix += d[v];
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
