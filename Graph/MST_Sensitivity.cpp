// SPDX-License-Identifier: CC0-1.0
// 参考（1辺の変更と代替辺）: https://atcoder.jp/contests/abc301/editorial/6353
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/mst_sensitivity.md
//
// API一覧
// minimum_spanning_forest_sensitivity(n,edges): 各辺を1本だけ除外または強制したときの交換辺を求める。
// SpanningForestSensitivity{forest,exchange}: 選んだ森の辺番号と、全入力辺に対応するoptionalの交換辺番号。
// 森に含まれるeでは、eを除外するとき代わりに入れる最小重みの非木辺。なければnullopt。
// 森に含まれないeでは、eを強制するとき除く木辺。自己辺は強制できないのでnullopt。
// 同じ重みの交換候補では入力番号が小さいもの。非連結・多重辺・負の重みも可。重みの総和は計算しない。
// O(n log(n+1)+m(log(m+1)+log(n+1)))時間・O(n log(n+1)+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MINIMUM_SPANNING_TREE_DEFINED
#define SIRKIT_MINIMUM_SPANNING_TREE_DEFINED

struct SpanningTreeEdge
{
    int from;
    int to;
    std::int64_t weight;
};

namespace detail
{
inline void SortSpanningEdgeWeights(std::vector<std::uint64_t> &keys, int id_bits, int weight_bits)
{
    constexpr unsigned kDigitBits = 8;
    constexpr std::size_t kBuckets = 1U << kDigitBits;
    constexpr auto kDigitMask = kBuckets - 1;
    std::vector<std::uint64_t> temporary(keys.size());
    for (int offset = 0; offset < weight_bits; offset += kDigitBits)
    {
        const int shift = offset + id_bits;
        std::array<std::size_t, kBuckets> count{};
        for (auto key : keys)
        {
            ++count[(key >> shift) & kDigitMask];
        }
        std::size_t seen = 0;
        for (auto &bucket : count)
        {
            const auto frequency = bucket;
            bucket = seen;
            seen += frequency;
        }
        for (auto key : keys)
        {
            temporary[count[(key >> shift) & kDigitMask]++] = key;
        }
        keys.swap(temporary);
    }
}
} // namespace detail

inline std::vector<std::size_t> minimum_spanning_forest(int n, std::span<const SpanningTreeEdge> edges)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::invalid_argument("minimum_spanning_forest: negative vertex count");
    }
#endif
    if (edges.empty())
    {
        return {};
    }

    constexpr std::size_t kRadixThreshold = 1024;
    const bool try_radix = edges.size() >= kRadixThreshold;
    auto minimum = edges.front().weight, maximum = minimum;
#ifdef SKIP_RUNTIME_CHECKS
    if (try_radix)
#endif
    {
        for (const auto &[from, to, weight] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (from < 0 || from >= n || to < 0 || to >= n)
            {
                throw std::out_of_range("minimum_spanning_forest: edge endpoint");
            }
#endif
            minimum = std::min(minimum, weight);
            maximum = std::max(maximum, weight);
        }
    }

    std::vector<std::size_t> result;
    if (n < 2)
    {
        return result;
    }
    std::vector<std::uint64_t> order(edges.size());
    auto id_mask = std::numeric_limits<std::uint64_t>::max();
    const auto unsigned_min = static_cast<std::uint64_t>(minimum);
    const auto weight_range = static_cast<std::uint64_t>(maximum) - unsigned_min;
    const int id_bits = std::bit_width(edges.size() - 1);
    const int weight_bits = std::bit_width(weight_range);
    if (try_radix && weight_bits <= 64 - id_bits)
    {
        if (weight_range == 0)
        {
            std::iota(order.begin(), order.end(), std::uint64_t{0});
        }
        else
        {
            // 下位ビットに辺番号を載せ、上位の重みだけを安定に並べる。
            id_mask = (std::uint64_t{1} << id_bits) - 1;
            for (std::size_t i = 0; i < order.size(); ++i)
            {
                order[i] = ((static_cast<std::uint64_t>(edges[i].weight) - unsigned_min) << id_bits) | i;
            }
            detail::SortSpanningEdgeWeights(order, id_bits, weight_bits);
        }
    }
    else
    {
        std::iota(order.begin(), order.end(), std::uint64_t{0});
        std::sort(order.begin(), order.end(), [&](auto a, auto b)
                  {
                      const auto left = edges[static_cast<std::size_t>(a)].weight;
                      const auto right = edges[static_cast<std::size_t>(b)].weight;
                      return left == right ? a < b : left < right;
                  });
    }

    std::vector<int> components(n, -1);
    const auto root = [&components](int vertex)
    {
        while (components[vertex] >= 0)
        {
            const int parent = components[vertex];
            if (components[parent] < 0)
            {
                return parent;
            }
            vertex = components[vertex] = components[parent];
        }
        return vertex;
    };
    const auto limit = static_cast<std::size_t>(n - 1);
    result.reserve(std::min(limit, edges.size()));
    for (auto key : order)
    {
        const auto id = static_cast<std::size_t>(key & id_mask);
        const auto &edge = edges[id];
        int a = root(edge.from), b = root(edge.to);
        if (a == b)
        {
            continue;
        }
        if (components[a] > components[b])
        {
            std::swap(a, b);
        }
        components[a] += components[b];
        components[b] = a;
        result.push_back(id);
        if (result.size() == limit)
        {
            break;
        }
    }
    return result;
}

#endif

#ifndef SIRKIT_MST_SENSITIVITY_DEFINED
#define SIRKIT_MST_SENSITIVITY_DEFINED

struct SpanningForestSensitivity
{
    std::vector<std::size_t> forest;
    std::vector<std::optional<std::size_t>> exchange;
};

namespace detail
{
class SpanningForestPathMaximum
{
public:
    SpanningForestPathMaximum(int n, std::span<const SpanningTreeEdge> edges,
                              const std::vector<std::size_t> &ids)
        : levels_(std::bit_width(static_cast<unsigned int>(n))), edges_(edges),
          depth_(n, -1), parent_edge_(n), jumps_(static_cast<std::size_t>(n) * levels_)
    {
        std::vector<std::vector<std::pair<int, std::size_t>>> adjacency(n);
        for (const auto id : ids)
        {
            const auto &[from, to, weight] = edges[id];
            adjacency[from].emplace_back(to, id);
            adjacency[to].emplace_back(from, id);
        }
        std::vector<int> order;
        order.reserve(n);
        for (int root = 0; root < n; ++root)
        {
            if (depth_[root] != -1)
            {
                continue;
            }
            depth_[root] = 0;
            std::fill_n(jumps_.begin() + offset(root), levels_, Jump{root, -1});
            auto cursor = order.size();
            order.push_back(root);
            for (; cursor < order.size(); ++cursor)
            {
                const int v = order[cursor];
                for (const auto &[u, edge] : adjacency[v])
                {
                    if (depth_[u] == -1)
                    {
                        depth_[u] = depth_[v] + 1;
                        parent_edge_[u] = edge;
                        auto *row = jumps_.data() + offset(u);
                        row[0] = {v, u};
                        for (unsigned int level = 1; level < levels_; ++level)
                        {
                            const auto low = row[level - 1];
                            const auto high = jump(low.ancestor, level - 1);
                            row[level] = {high.ancestor, maximum(low.heaviest_child, high.heaviest_child)};
                        }
                        order.push_back(u);
                    }
                }
            }
        }
    }

    [[nodiscard]] int depth(int v) const noexcept { return depth_[v]; }
    [[nodiscard]] int parent(int v) const noexcept { return jump(v, 0).ancestor; }
    [[nodiscard]] std::size_t parent_edge(int v) const noexcept { return parent_edge_[v]; }

    std::pair<int, int> query(int u, int v) const noexcept
    {
        int maximum_child = -1;
        if (depth_[u] < depth_[v])
        {
            std::swap(u, v);
        }
        auto difference = static_cast<unsigned int>(depth_[u] - depth_[v]);
        for (; difference != 0; difference &= difference - 1)
        {
            const auto up = jump(u, static_cast<unsigned int>(std::countr_zero(difference)));
            maximum_child = maximum(maximum_child, up.heaviest_child);
            u = up.ancestor;
        }
        if (u == v)
        {
            return {u, maximum_child};
        }
        for (auto level = levels_; level != 0; )
        {
            --level;
            const auto a = jump(u, level), b = jump(v, level);
            if (a.ancestor != b.ancestor)
            {
                maximum_child = maximum(maximum_child, maximum(a.heaviest_child, b.heaviest_child));
                u = a.ancestor;
                v = b.ancestor;
            }
        }
        return {parent(u), maximum(maximum_child, maximum(u, v))};
    }

private:
    struct Jump
    {
        int ancestor;
        int heaviest_child;
    };

    [[nodiscard]] std::size_t offset(int v) const noexcept
    {
        return static_cast<std::size_t>(v) * levels_;
    }

    [[nodiscard]] Jump jump(int v, unsigned int level) const noexcept
    {
        return jumps_[offset(v) + level];
    }

    [[nodiscard]] int maximum(int a, int b) const noexcept
    {
        if (a == -1)
        {
            return b;
        }
        if (b == -1)
        {
            return a;
        }
        const auto left = parent_edge_[a], right = parent_edge_[b];
        const auto wa = edges_[left].weight, wb = edges_[right].weight;
        return (wa == wb ? left < right : wa > wb) ? a : b;
    }

    unsigned int levels_;
    std::span<const SpanningTreeEdge> edges_;
    std::vector<int> depth_;
    std::vector<std::size_t> parent_edge_;
    std::vector<Jump> jumps_;
};

class SpanningForestUnassigned
{
public:
    explicit SpanningForestUnassigned(int n) : groups_(n, -1), next_(n)
    {
        std::iota(next_.begin(), next_.end(), 0);
    }

    int find(int v)
    {
        return next_[leader(v)];
    }

    int consume(int v, int parent)
    {
        int a = leader(v), b = leader(parent);
        const int next = next_[b];
        if (groups_[a] > groups_[b])
        {
            std::swap(a, b);
        }
        groups_[a] += groups_[b];
        groups_[b] = a;
        next_[a] = next;
        return next;
    }

private:
    int leader(int v)
    {
        auto root = v;
        while (groups_[root] >= 0)
        {
            root = groups_[root];
        }
        while (v != root)
        {
            const int parent = groups_[v];
            groups_[v] = root;
            v = parent;
        }
        return root;
    }

    std::vector<int> groups_, next_;
};
}

inline SpanningForestSensitivity minimum_spanning_forest_sensitivity(
    int n, std::span<const SpanningTreeEdge> edges)
{
    if (n < 0)
    {
        throw std::length_error("minimum_spanning_forest_sensitivity: negative vertex count");
    }
    SpanningForestSensitivity result{minimum_spanning_forest(n, edges),
                                     std::vector<std::optional<std::size_t>>(edges.size())};
    std::vector<std::uint8_t> in_tree(edges.size());
    for (const auto id : result.forest)
    {
        in_tree[id] = 1;
    }
    std::vector<std::size_t> candidates;
    candidates.reserve(edges.size() - result.forest.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (!in_tree[i] && edges[i].from != edges[i].to)
        {
            candidates.push_back(i);
        }
    }
    if (candidates.empty())
    {
        return result;
    }
    std::sort(candidates.begin(), candidates.end(), [&](std::size_t a, std::size_t b)
              {
                  const auto left = edges[a].weight, right = edges[b].weight;
                  return left == right ? a < b : left < right;
              });
    const detail::SpanningForestPathMaximum tree(n, edges, result.forest);
    detail::SpanningForestUnassigned unassigned(n);
    for (const auto id : candidates)
    {
        const auto &[u, v, weight] = edges[id];
        const auto [ancestor, heaviest] = tree.query(u, v);
        result.exchange[id] = tree.parent_edge(heaviest);
        // 最初に辺を通る非木辺が最小の代替辺。割り当てた木辺は次の照会から飛ばす。
        for (const int endpoint : {u, v})
        {
            int x = unassigned.find(endpoint);
            while (tree.depth(x) > tree.depth(ancestor))
            {
                result.exchange[tree.parent_edge(x)] = id;
                x = unassigned.consume(x, tree.parent(x));
            }
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
