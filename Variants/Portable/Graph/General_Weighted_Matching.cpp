// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/General_Weighted_Matching.cpp
// 参考（重み付きBlossom法）: https://doi.org/10.1145/6462.6502
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/general_weighted_matching.md
//
// API一覧
// maximum_weight_matching(n, edges): 無向グラフから辺の重みの総和が最大のマッチングを1つ返す。
// GeneralWeightedMatchingEdge{u,v,weight}: [0,n)の端点と符号付き64ビットの重み。
// GeneralWeightedMatchingResult{weight,edges}: 重みの総和と選んだ端点の組。自己辺と非正の辺は選ばない。
// 多重辺は同じ端点間の最大重みを使う。最適解が複数ある場合の組や個数は規定しない。
// n頂点・m辺に対しO(n^3+m)時間・O(n^2)領域。正の辺重みの上限はint64_t::max()/4。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GENERAL_WEIGHTED_MATCHING_DEFINED
#define SIRKIT_GENERAL_WEIGHTED_MATCHING_DEFINED

struct GeneralWeightedMatchingEdge
{
    int u;
    int v;
    std::int64_t weight;
};

struct GeneralWeightedMatchingResult
{
    std::int64_t weight = 0;
    std::vector<std::pair<int, int>> edges;
};

namespace detail
{
class WeightedMatchingSearch
{
public:
    WeightedMatchingSearch(int n, std::span<const GeneralWeightedMatchingEdge> edges)
        : n_(n), last_(n),
          edges_(2 * n + 1, std::vector<Arc>(2 * n + 1)),
          weights_(n + 1, std::vector<std::int64_t>(n + 1)),
          child_for_(2 * n + 1, std::vector<int>(n + 1)),
          cycle_(2 * n + 1), base_(2 * n + 1), top_(2 * n + 1), mate_(n + 1),
          side_(2 * n + 1), trace_(2 * n + 1),
          least_slack_(2 * n + 1), vertex_price_(n + 1), blossom_price_(2 * n + 1), seen_(2 * n + 1)
    {
        std::int64_t max_weight = 0;
        for (const auto &[u, v, w] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (u < 0 || u >= n || v < 0 || v >= n)
            {
                throw std::out_of_range("maximum_weight_matching: edge endpoint");
            }
#endif
            if (u == v || w <= 0)
            {
                continue;
            }
            // 価格は各頂点で高々最大重みの2倍。2頂点分の和を保持する。
            if (w > std::numeric_limits<std::int64_t>::max() / 4)
            {
                throw std::overflow_error("maximum_weight_matching: edge weight too large");
            }
            const int a = u + 1, b = v + 1;
            if (w > weights_[a][b])
            {
                weights_[a][b] = weights_[b][a] = w;
                edges_[a][b] = {a, b};
                edges_[b][a] = {b, a};
                max_weight = std::max(max_weight, w);
            }
        }
        for (int v = 1; v <= n_; ++v)
        {
            top_[v] = base_[v] = v;
            child_for_[v][v] = v;
            vertex_price_[v] = max_weight;
        }
        pending_.reserve(n);
    }

    GeneralWeightedMatchingResult solve()
    {
        while (augment_once())
        {
        }
        GeneralWeightedMatchingResult result;
        for (int v = 1; v <= n_; ++v)
        {
            if (const int u = mate_[v]; u > v)
            {
                const auto weight = weights_[v][u];
                if (result.weight > std::numeric_limits<std::int64_t>::max() - weight)
                {
                    throw std::overflow_error("maximum_weight_matching: total weight overflow");
                }
                result.weight += weight;
                result.edges.emplace_back(v - 1, u - 1);
            }
        }
        return result;
    }

private:
    struct Arc
    {
        int from = 0;
        int to = 0;
    };

    enum class Side : unsigned char
    {
        unseen,
        outer,
        inner
    };

    static Arc reverse(Arc edge) noexcept
    {
        return {edge.to, edge.from};
    }

    std::int64_t slack(Arc edge) const noexcept
    {
        return vertex_price_[edge.from] + vertex_price_[edge.to] - 2 * weights_[edge.from][edge.to];
    }

    void consider(int component, Arc edge)
    {
        auto &saved = least_slack_[component];
        if (saved.from == 0 || slack(edge) < slack(saved))
        {
            saved = edge;
        }
    }

    void recompute_slack(int component)
    {
        least_slack_[component] = {};
        if (side_[component] == Side::inner)
        {
            return;
        }
        for (int v = 1; v <= n_; ++v)
        {
            const auto edge = edges_[v][component];
            if (top_[v] != component && side_[top_[v]] == Side::outer && edge.from != 0)
            {
                consider(component, edge);
            }
        }
    }

    void enqueue(int component)
    {
        if (component <= n_)
        {
            pending_.push_back(component);
            return;
        }
        for (const int child : cycle_[component])
        {
            enqueue(child);
        }
    }

    void set_top(int component, int top)
    {
        top_[component] = top;
        if (component > n_)
        {
            for (const int child : cycle_[component])
            {
                set_top(child, top);
            }
        }
    }

    int partner(int component) const noexcept
    {
        return top_[mate_[base_[component]]];
    }

    int forest_parent(int outer) const noexcept
    {
        // 根ならpartner=0で、trace_[0]は常に空の辺。
        return top_[trace_[partner(outer)].from];
    }

    int shared_root(int a, int b)
    {
        ++epoch_;
        while (a != 0 || b != 0)
        {
            if (a != 0)
            {
                if (seen_[a] == epoch_)
                {
                    return a;
                }
                seen_[a] = epoch_;
                a = forest_parent(a);
            }
            std::swap(a, b);
        }
        return 0;
    }

    std::size_t orient_cycle(int blossom, int child)
    {
        auto &cycle = cycle_[blossom];
        const auto position = static_cast<std::size_t>(std::find(cycle.begin(), cycle.end(), child) - cycle.begin());
        if ((position & 1) == 0)
        {
            return position;
        }
        std::reverse(cycle.begin() + 1, cycle.end());
        return cycle.size() - position;
    }

    void attach(int component, Arc edge)
    {
        if (component <= n_)
        {
            mate_[component] = edge.to;
            return;
        }
        // 新しい外部辺の端点まで、偶数長の側で閉路内の組を切り替える。
        auto &cycle = cycle_[component];
        const int child = child_for_[component][edge.from];
        const auto position = orient_cycle(component, child);
        for (std::size_t i = 0; i < position; i += 2)
        {
            const auto chosen = edges_[cycle[i]][cycle[i + 1]];
            attach(cycle[i], chosen);
            attach(cycle[i + 1], reverse(chosen));
        }
        attach(child, edge);
        std::rotate(cycle.begin(), cycle.begin() + position, cycle.end());
        base_[component] = edge.from;
    }

    void augment_side(int component, Arc edge)
    {
        for (;;)
        {
            const int previous = partner(component);
            attach(component, edge);
            if (previous == 0)
            {
                return;
            }
            edge = trace_[previous];
            attach(previous, reverse(edge));
            component = top_[edge.from];
        }
    }

    void collect_branch(std::vector<int> &cycle, int component, int base)
    {
        for (; component != base; component = forest_parent(component))
        {
            const int inner = partner(component);
            cycle.push_back(component);
            cycle.push_back(inner);
            enqueue(inner);
        }
    }

    void shrink(int a, int b, int base)
    {
        int blossom = n_ + 1;
        while (blossom <= last_ && top_[blossom] != 0)
        {
            ++blossom;
        }
        last_ = std::max(last_, blossom);
        auto &cycle = cycle_[blossom];
        cycle.clear();
        cycle.push_back(base);
        collect_branch(cycle, a, base);
        std::reverse(cycle.begin() + 1, cycle.end());
        collect_branch(cycle, b, base);
        top_[blossom] = blossom;
        base_[blossom] = base_[base];
        side_[blossom] = Side::outer;
        blossom_price_[blossom] = 0;
        for (const int child : cycle)
        {
            set_top(child, blossom);
        }

        // 別成分への辺は、換算費用が最小の実辺だけを成分間の代表にする。
        for (int other = 1; other <= last_; ++other)
        {
            if (other == blossom)
            {
                continue;
            }
            Arc best;
            for (const int child : cycle)
            {
                const auto edge = edges_[child][other];
                if (edge.from != 0 && (best.from == 0 || slack(edge) < slack(best)))
                {
                    best = edge;
                }
            }
            edges_[blossom][other] = best;
            edges_[other][blossom] = reverse(best);
        }
        std::fill(child_for_[blossom].begin(), child_for_[blossom].end(), 0);
        for (const int child : cycle)
        {
            for (int v = 1; v <= n_; ++v)
            {
                if (child_for_[child][v] != 0)
                {
                    child_for_[blossom][v] = child;
                }
            }
        }
        recompute_slack(blossom);
    }

    void expand(int blossom)
    {
        const auto entering = trace_[blossom];
        const int first = child_for_[blossom][entering.to];
        const auto position = orient_cycle(blossom, first);
        const auto &cycle = cycle_[blossom];
        for (const int child : cycle)
        {
            set_top(child, child);
            side_[child] = Side::unseen;
        }
        // 入口から旧基点までを交互木へ戻し、閉路の残りは未探索にする。
        side_[first] = Side::inner;
        trace_[first] = entering;
        for (std::size_t i = 0; i < position; i += 2)
        {
            const int inner = cycle[i], outer = cycle[i + 1];
            side_[inner] = Side::inner;
            side_[outer] = Side::outer;
            trace_[inner] = edges_[outer][inner];
            enqueue(outer);
        }
        top_[blossom] = 0;
        for (const int child : cycle)
        {
            recompute_slack(child);
        }
    }

    bool use_tight(Arc edge)
    {
        const int a = top_[edge.from], b = top_[edge.to];
        if (side_[b] == Side::unseen)
        {
            trace_[b] = edge;
            side_[b] = Side::inner;
            const int other = partner(b);
            side_[other] = Side::outer;
            least_slack_[other] = {};
            enqueue(other);
            return false;
        }
        const int base = shared_root(a, b);
        if (base != 0)
        {
            shrink(a, b, base);
            return false;
        }
        augment_side(a, edge);
        augment_side(b, reverse(edge));
        return true;
    }

    bool augment_once()
    {
        std::fill(side_.begin(), side_.end(), Side::unseen);
        std::fill(least_slack_.begin(), least_slack_.end(), Arc{});
        pending_.clear();
        for (int i = 1; i <= last_; ++i)
        {
            if (top_[i] == i && partner(i) == 0)
            {
                side_[i] = Side::outer;
                enqueue(i);
            }
        }
        std::size_t front = 0;
        for (;;)
        {
            while (front < pending_.size())
            {
                const int v = pending_[front++];
                for (int u = 1; u <= n_; ++u)
                {
                    const auto edge = edges_[v][u];
                    const int component = top_[u];
                    if (edge.from == 0 || top_[v] == component || side_[component] == Side::inner)
                    {
                        continue;
                    }
                    if (slack(edge) == 0)
                    {
                        if (use_tight(edge))
                        {
                            return true;
                        }
                    }
                    else
                    {
                        consider(component, edge);
                    }
                }
            }

            auto free_price = std::numeric_limits<std::int64_t>::max();
            for (int v = 1; v <= n_; ++v)
            {
                if (mate_[v] == 0)
                {
                    free_price = std::min(free_price, vertex_price_[v]);
                }
            }
            auto delta = free_price;
            for (int i = 1; i <= last_; ++i)
            {
                if (top_[i] != i)
                {
                    continue;
                }
                if (side_[i] == Side::inner && i > n_)
                {
                    delta = std::min(delta, blossom_price_[i] / 2);
                }
                const auto edge = least_slack_[i];
                if (side_[i] != Side::inner && edge.from != 0)
                {
                    const auto factor = side_[i] == Side::outer ? 2 : 1;
                    delta = std::min(delta, slack(edge) / factor);
                }
            }
            // 未マッチ頂点の価格が0になれば、いまのマッチングが最適になる。
            if (delta == free_price)
            {
                return false;
            }
            for (int v = 1; v <= n_; ++v)
            {
                if (side_[top_[v]] == Side::outer)
                {
                    vertex_price_[v] -= delta;
                }
                else if (side_[top_[v]] == Side::inner)
                {
                    vertex_price_[v] += delta;
                }
            }
            for (int i = n_ + 1; i <= last_; ++i)
            {
                if (top_[i] != i)
                {
                    continue;
                }
                if (side_[i] == Side::outer)
                {
                    blossom_price_[i] += 2 * delta;
                }
                else if (side_[i] == Side::inner)
                {
                    blossom_price_[i] -= 2 * delta;
                }
            }
            for (int i = 1; i <= last_; ++i)
            {
                const auto edge = least_slack_[i];
                // 縮約・展開で新しく最上位になった成分は最小換算費用を再計算済み。
                if (top_[i] == i && side_[i] != Side::inner && edge.from != 0 && slack(edge) == 0)
                {
                    if (use_tight(edge))
                    {
                        return true;
                    }
                }
            }
            for (int i = n_ + 1; i <= last_; ++i)
            {
                if (top_[i] == i && side_[i] == Side::inner && blossom_price_[i] == 0)
                {
                    expand(i);
                }
            }
        }
    }

    int n_, last_;
    std::vector<std::vector<Arc>> edges_;
    std::vector<std::vector<std::int64_t>> weights_;
    std::vector<std::vector<int>> child_for_, cycle_;
    std::vector<int> base_, top_, mate_;
    std::vector<Side> side_;
    std::vector<Arc> trace_, least_slack_;
    std::vector<std::int64_t> vertex_price_, blossom_price_;
    std::vector<std::size_t> seen_;
    std::size_t epoch_ = 0;
    std::vector<int> pending_;
};
}

inline GeneralWeightedMatchingResult maximum_weight_matching(int n, std::span<const GeneralWeightedMatchingEdge> edges)
{
    if (n < 0 || n > std::numeric_limits<int>::max() / 2)
    {
        throw std::length_error("maximum_weight_matching: vertex count");
    }
    return detail::WeightedMatchingSearch(n, edges).solve();
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
