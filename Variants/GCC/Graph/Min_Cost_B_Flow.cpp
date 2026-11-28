// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Min_Cost_B_Flow.cpp
// 参考（費用スケーリング）: https://doi.org/10.1287/moor.15.3.430
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/min_cost_b_flow.md
//
// API一覧
// BFlowEdge{from,to,lower,upper,cost}: 流量範囲[lower,upper]の有向辺。費用や下限に負を許す。
// min_cost_b_flow(supply,edges): 各頂点で出る流量−入る流量=supplyを満たす最小費用解を返す。
// BFlowResult{flow,potential}: 入力順の辺流量と頂点ポテンシャル。実現不能はnullopt。
// 逆向きも含む辺範囲と内部の費用・流量の演算はint64_tに収める。総費用の型は利用側で選ぶ。
// n頂点、m辺、辺費用の最大絶対値C。最大流と費用スケーリングでO((n+1)^2(n+m+1)(1+log(1+nC)))時間・O(n+m+1)領域。
// 下限・上限や端点の入力検査はSKIP_RUNTIME_CHECKSで省ける。添字・算術の上限検査は維持する。
// 同梱のmf_graph<Cap>: 最大流のAPIと計算量はDocument/max_flow.mdを参照。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <new>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MAX_FLOW_CAPACITY_DEFINED
#define SIRKIT_MAX_FLOW_CAPACITY_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline void CheckMaxFlowAppendCapacity(std::size_t edges, std::size_t from_degree,
                                       std::size_t to_degree, bool loop) {
    constexpr auto maximum = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if (edges >= maximum) throw std::length_error("mf_graph: too many edges");
    if (from_degree >= maximum || to_degree >= maximum || (loop && from_degree >= maximum - 1))
        throw std::length_error("mf_graph: too many incident edges");
}

constexpr bool MaxFlowCanUsePreflow(std::size_t vertices) noexcept {
    return vertices < static_cast<std::size_t>(std::numeric_limits<int>::max());
}
}
#endif
#ifndef SIRKIT_MAX_FLOW_DEFINED
#define SIRKIT_MAX_FLOW_DEFINED

template<class Cap = std::int64_t>
class mf_graph {
    static_assert(std::is_integral_v<Cap> && !std::is_same_v<Cap, bool>, "capacity must be a non-bool integer");
public:
    struct edge { int from; int to; Cap cap; Cap flow; };
private:
    struct Arc { int to; int reverse; Cap residual; };
    struct Frame { int vertex; Cap limit; Cap sent; };
    std::vector<std::vector<Arc>> graph_;
    std::vector<std::pair<int, int>> positions_;
    std::vector<int> level_, next_, queue_;
    std::vector<Frame> frames_;
    static std::size_t checked_size(int n) {
        if (n < 0) throw std::length_error("mf_graph: negative vertex count");
        return static_cast<std::size_t>(n);
    }
#ifndef SKIP_RUNTIME_CHECKS
    void vertex(int v) const {
        if (v < 0 || v >= size()) throw std::out_of_range("mf_graph: vertex out of range");
    }
    void edge_id(int id) const {
        if (id < 0 || static_cast<std::size_t>(id) >= positions_.size()) throw std::out_of_range("mf_graph: edge out of range");
    }
    static void nonnegative(Cap value) {
        if constexpr (std::is_signed_v<Cap>) if (value < 0) throw std::invalid_argument("mf_graph: negative capacity");
    }
#endif
    bool levels(int source, int sink) {
        std::fill(level_.begin(), level_.end(), -1);
        int front = 0, end = 0; queue_[end++] = source; level_[source] = 0;
        while (front != end) {
            const int v = queue_[front++];
            for (const auto& edge : graph_[v]) if (edge.residual != 0 && level_[edge.to] == -1) {
                level_[edge.to] = level_[v] + 1;
                // sinkの直前層までは発見済みなので、最短路の辺はDFSで列挙できる。
                if (edge.to == sink) return true;
                queue_[end++] = edge.to;
            }
        }
        return false;
    }
    Cap blocking_flow(int source, int sink, Cap limit) {
        frames_.clear(); frames_.push_back({source, limit, 0});
        for (;;) {
            auto& frame = frames_.back(); const int v = frame.vertex;
            if (v != sink && frame.sent != frame.limit) {
                auto& cursor = next_[v];
                while (cursor != static_cast<int>(graph_[v].size())) {
                    const auto& edge = graph_[v][cursor];
                    if (edge.residual != 0 && level_[edge.to] == level_[v] + 1) break;
                    ++cursor;
                }
                if (cursor != static_cast<int>(graph_[v].size())) {
                    const auto& edge = graph_[v][cursor];
                    frames_.push_back({edge.to, std::min(static_cast<Cap>(frame.limit - frame.sent), edge.residual), 0});
                    continue;
                }
                level_[v] = -1;
            }
            const Cap sent = v == sink ? frame.limit : frame.sent;
            frames_.pop_back();
            if (frames_.empty()) return sent;
            auto& parent = frames_.back(); auto& edge = graph_[parent.vertex][next_[parent.vertex]];
            edge.residual -= sent; graph_[edge.to][edge.reverse].residual += sent;
            parent.sent += sent;
            if (edge.residual == 0 || level_[edge.to] == -1) ++next_[parent.vertex];
        }
    }
    Cap unit_flow(int source, int sink) {
        std::fill(level_.begin(), level_.end(), -1);
        int front = 0, tail = 0;
        queue_[tail++] = source;
        level_[source] = source;
        while (front != tail) {
            const int v = queue_[front++];
            for (int index = 0; index != static_cast<int>(graph_[v].size()); ++index) {
                const auto& edge = graph_[v][index];
                const int u = edge.to;
                if (edge.residual == 0 || level_[u] != -1) continue;
                level_[u] = v;
                next_[u] = index;
                if (u == sink) {
                    for (int to = sink; to != source; to = level_[to]) {
                        auto& arc = graph_[level_[to]][next_[to]];
                        --arc.residual;
                        ++graph_[to][arc.reverse].residual;
                    }
                    return Cap{1};
                }
                queue_[tail++] = u;
            }
        }
        return Cap{0};
    }
    Cap preflow(int source, int sink, Cap limit) {
        const std::size_t n = graph_.size();
        const std::uint32_t virtual_height = static_cast<std::uint32_t>(n + 1);
        constexpr std::uint32_t unseen = std::numeric_limits<std::uint32_t>::max();
        // 確保失敗時は、残余を変更する前に退出する。
        std::vector<std::uint32_t> height(n);
        std::vector<Cap> excess(n, Cap{0});
        std::vector<int> cursor(n, 0), fifo(n);
        std::vector<unsigned char> active(n, 0);
        std::size_t front = 0, back = 0, count = 0;
        auto enqueue = [&](int v) {
            if (v == sink || active[v] || excess[v] == 0) return;
            active[v] = 1;
            fifo[back] = v;
            if (++back == n) back = 0;
            ++count;
        };
        auto relabel_all = [&] {
            // activeは再構築中だけBFSの訪問印として使う。
            std::fill(active.begin(), active.end(), 0);
            auto assign_height = [&](int v, std::uint32_t value) {
                if (height[v] != value) cursor[v] = 0;
                height[v] = value;
                active[v] = 1;
            };
            int head = 0, tail = 0;
            queue_[tail++] = sink;
            assign_height(sink, 0);
            while (head != tail) {
                const int v = queue_[head++];
                for (const auto& edge : graph_[v]) {
                    const int u = edge.to;
                    if (!active[u] && graph_[u][edge.reverse].residual != 0) {
                        assign_height(u, height[v] + 1);
                        queue_[tail++] = u;
                    }
                }
            }
            // tへ届かない余剰も、s経由で仮想始点へ戻して流量保存を回復する。
            if (!active[source]) {
                head = tail = 0;
                assign_height(source, virtual_height + 1);
                queue_[tail++] = source;
                while (head != tail) {
                    const int v = queue_[head++];
                    for (const auto& edge : graph_[v]) {
                        const int u = edge.to;
                        if (!active[u] && graph_[u][edge.reverse].residual != 0) {
                            assign_height(u, height[v] + 1);
                            queue_[tail++] = u;
                        }
                    }
                }
            }
            for (std::size_t v = 0; v < n; ++v) {
                if (!active[v] && height[v] != unseen) {
                    height[v] = unseen;
                    cursor[v] = 0;
                }
                active[v] = 0;
            }
            front = back = count = 0;
            for (int v = 0; v < static_cast<int>(n); ++v) enqueue(v);
        };
        excess[source] = limit;
        relabel_all();
        std::uint64_t work = 0;
        const std::uint64_t budget = static_cast<std::uint64_t>(positions_.size()) + n;
        while (count != 0) {
            const int v = fifo[front];
            if (++front == n) front = 0;
            --count;
            active[v] = 0;
            for (;;) {
                if (v == source && height[v] >= virtual_height + 1) {
                    // 容量limitの暗黙の逆辺で、未使用予算を返す。
                    excess[v] = 0;
                    break;
                }
                const int degree = static_cast<int>(graph_[v].size());
                int& index = cursor[v];
                while (index != degree) {
                    auto& edge = graph_[v][index];
                    ++work;
                    if (edge.residual != 0 && height[edge.to] != unseen &&
                        height[v] == height[edge.to] + 1) {
                        const Cap amount = std::min(excess[v], edge.residual);
                        edge.residual -= amount;
                        graph_[edge.to][edge.reverse].residual += amount;
                        excess[v] -= amount;
                        excess[edge.to] += amount;
                        enqueue(edge.to);
                        if (excess[v] == 0) break;
                    }
                    ++index;
                }
                if (excess[v] == 0) break;
                std::uint32_t best = v == source ? virtual_height : unseen;
                for (const auto& edge : graph_[v]) {
                    if (edge.residual != 0) best = std::min(best, height[edge.to]);
                }
                work += graph_[v].size();
                // 正の余剰には、流した経路の逆辺で仮想始点へ戻る経路が必ずある。
                height[v] = best + 1;
                index = 0;
                if (work >= budget) break;
            }
            if (work >= budget) {
                relabel_all();
                work = 0;
            }
        }
        return excess[sink];
    }
public:
    mf_graph() = default;
    explicit mf_graph(int n) : graph_(checked_size(n)), level_(graph_.size()), next_(graph_.size()), queue_(graph_.size()) {}
    mf_graph(const mf_graph&) = default;
    mf_graph(mf_graph&& other) noexcept { swap(other); }
    mf_graph& operator=(mf_graph other) noexcept { swap(other); return *this; }
    void swap(mf_graph& other) noexcept {
        graph_.swap(other.graph_); positions_.swap(other.positions_); level_.swap(other.level_);
        next_.swap(other.next_); queue_.swap(other.queue_); frames_.swap(other.frames_);
    }
    void reset(int n = 0) { mf_graph replacement(n); swap(replacement); }
    int size() const noexcept { return static_cast<int>(graph_.size()); }
    std::size_t edge_count() const noexcept { return positions_.size(); }
    void reserve_edges(std::size_t count) {
        if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("mf_graph: too many edges");
        positions_.reserve(count);
    }
    int add_edge(int from, int to, Cap capacity) {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(from); vertex(to); nonnegative(capacity);
#endif
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckMaxFlowAppendCapacity(
            positions_.size(), graph_[from].size(), graph_[to].size(), from == to);
        const int forward = static_cast<int>(graph_[from].size());
        const int backward = static_cast<int>(graph_[to].size()) + static_cast<int>(from == to);
        graph_[from].push_back({to, backward, capacity});
        try { graph_[to].push_back({from, forward, 0}); }
        catch (...) { graph_[from].pop_back(); throw; }
        try { positions_.emplace_back(from, forward); }
        catch (...) { graph_[to].pop_back(); graph_[from].pop_back(); throw; }
        return static_cast<int>(positions_.size()) - 1;
    }
    edge get_edge(int id) { return std::as_const(*this).get_edge(id); }
    edge get_edge(int id) const {
#ifndef SKIP_RUNTIME_CHECKS
        edge_id(id);
#endif
        const auto [from, position] = positions_[id]; const auto& edge = graph_[from][position];
        const Cap sent = graph_[edge.to][edge.reverse].residual;
        return {from, edge.to, static_cast<Cap>(edge.residual + sent), sent};
    }
    std::vector<edge> edges() { return std::as_const(*this).edges(); }
    std::vector<edge> edges() const {
        std::vector<edge> result; result.reserve(positions_.size());
        for (int id = 0; id < static_cast<int>(positions_.size()); ++id) result.push_back(get_edge(id));
        return result;
    }
    void change_edge(int id, Cap capacity, Cap sent) {
#ifndef SKIP_RUNTIME_CHECKS
        edge_id(id); nonnegative(capacity); nonnegative(sent);
        if (sent > capacity) throw std::invalid_argument("mf_graph: flow exceeds capacity");
#endif
        const auto [from, position] = positions_[id]; auto& edge = graph_[from][position];
        edge.residual = capacity - sent; graph_[edge.to][edge.reverse].residual = sent;
    }
    Cap flow(int source, int sink) { return flow(source, sink, std::numeric_limits<Cap>::max()); }
    Cap flow(int source, int sink, Cap limit) {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(source); vertex(sink); nonnegative(limit);
#endif
        if (source == sink || limit == 0) return 0;
        if (limit == Cap{1}) return unit_flow(source, sink);
        frames_.reserve(graph_.size());
        Cap result = 0;
        int phases = 0;
        // 最大頂点数では高さの上限と未発見印が同じ値になるため、Dinicを続ける。
        bool allow_preflow = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::MaxFlowCanUsePreflow(graph_.size());
        while (result != limit && levels(source, sink)) {
            if (allow_preflow && phases >= 4) {
                try { return result + preflow(source, sink, static_cast<Cap>(limit - result)); }
                catch (const std::bad_alloc&) { allow_preflow = false; }
            }
            std::fill(next_.begin(), next_.end(), 0);
            result += blocking_flow(source, sink, static_cast<Cap>(limit - result));
            ++phases;
        }
        return result;
    }
    std::vector<bool> min_cut(int source) { return std::as_const(*this).min_cut(source); }
    std::vector<bool> min_cut(int source) const {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(source);
#endif
        std::vector<bool> reachable(graph_.size()); std::vector<int> queue;
        queue.reserve(graph_.size()); queue.push_back(source); reachable[source] = true;
        for (std::size_t front = 0; front < queue.size(); ++front) {
            const int v = queue[front];
            for (const auto& edge : graph_[v]) if (edge.residual != 0 && !reachable[edge.to]) {
                reachable[edge.to] = true; queue.push_back(edge.to);
            }
        }
        return reachable;
    }
};
#endif


#ifndef SIRKIT_MIN_COST_B_FLOW_DEFINED
#define SIRKIT_MIN_COST_B_FLOW_DEFINED

struct BFlowEdge
{
    int from;
    int to;
    std::int64_t lower;
    std::int64_t upper;
    std::int64_t cost;
};

struct BFlowResult
{
    std::vector<std::int64_t> flow;
    std::vector<std::int64_t> potential;
};

namespace detail
{
inline void check_b_flow_capacity(std::size_t n, std::size_t m)
{
    constexpr auto kMax = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if (n > kMax - 2 || m > kMax - n)
    {
        throw std::length_error("min_cost_b_flow: too many vertices or edges");
    }
}

inline std::int64_t b_flow_add(std::int64_t a, std::int64_t b)
{
    if ((b > 0 && a > std::numeric_limits<std::int64_t>::max() - b) ||
        (b < 0 && a < std::numeric_limits<std::int64_t>::min() - b))
    {
        throw std::overflow_error("min_cost_b_flow: addition overflow");
    }
    return a + b;
}

inline std::int64_t b_flow_subtract(std::int64_t a, std::int64_t b)
{
    if ((b > 0 && a < std::numeric_limits<std::int64_t>::min() + b) ||
        (b < 0 && a > std::numeric_limits<std::int64_t>::max() + b))
    {
        throw std::overflow_error("min_cost_b_flow: subtraction overflow");
    }
    return a - b;
}

class BFlowResidual
{
public:
    explicit BFlowResidual(int n) : graph_(n), potential_(n), excess_(n), cursor_(n) {}

    std::size_t add_edge(int from, int to, std::int64_t cap, std::int64_t flow, std::int64_t cost)
    {
        const auto forward = graph_[from].size(), reverse = graph_[to].size();
        graph_[from].push_back({to, reverse, cap - flow, cost});
        graph_[to].push_back({from, forward, flow, -cost});
        return forward;
    }

    [[nodiscard]] std::int64_t flow(int from, std::size_t index) const
    {
        const auto &edge = graph_[from][index];
        return graph_[edge.to][edge.reverse].capacity;
    }

    void optimize(std::int64_t epsilon)
    {
        while (epsilon > 1)
        {
            epsilon = std::max<std::int64_t>(1, epsilon / 4);
            refine(epsilon);
        }
    }

    [[nodiscard]] std::vector<std::int64_t> exact_potential(std::int64_t factor) const
    {
        // 残余には負閉路がないので、全頂点を始点とした最短距離が双対解になる。
        const int n = static_cast<int>(graph_.size());
        std::vector<std::int64_t> exact(n);
        for (int pass = 0; pass < n; ++pass)
        {
            bool changed = false;
            for (int v = 0; v < n; ++v)
            {
                for (const auto &edge : graph_[v])
                {
                    if (edge.capacity == 0)
                    {
                        continue;
                    }
                    const auto candidate = b_flow_add(exact[v], edge.cost / factor);
                    if (candidate < exact[edge.to])
                    {
                        exact[edge.to] = candidate;
                        changed = true;
                    }
                }
            }
            if (!changed)
            {
                break;
            }
        }
        return exact;
    }

private:
    struct Arc
    {
        int to;
        std::size_t reverse;
        std::int64_t capacity;
        std::int64_t cost;
    };

    [[nodiscard]] std::int64_t reduced_cost(int v, const Arc &edge) const
    {
        const auto from = potential_[v], to = potential_[edge.to];
        if ((from < 0) == (to < 0))
        {
            return b_flow_add(edge.cost, b_flow_subtract(from, to));
        }
        return b_flow_subtract(b_flow_add(edge.cost, from), to);
    }

    void push(int from, Arc &edge, std::int64_t amount)
    {
        excess_[from] = b_flow_subtract(excess_[from], amount);
        excess_[edge.to] = b_flow_add(excess_[edge.to], amount);
        edge.capacity -= amount;
        graph_[edge.to][edge.reverse].capacity += amount;
    }

    void refine(std::int64_t epsilon)
    {
        const int n = static_cast<int>(graph_.size());
        std::deque<int> active;
        for (int v = 0; v < n; ++v)
        {
            for (auto &edge : graph_[v])
            {
                if (edge.capacity > 0 && reduced_cost(v, edge) < 0)
                {
                    push(v, edge, edge.capacity);
                }
            }
        }
        for (int v = 0; v < n; ++v)
        {
            if (excess_[v] > 0)
            {
                active.push_back(v);
            }
        }
        std::fill(cursor_.begin(), cursor_.end(), 0);
        while (!active.empty())
        {
            const int v = active.front();
            active.pop_front();
            auto &index = cursor_[v];
            while (excess_[v] > 0)
            {
                if (index == graph_[v].size())
                {
                    // 余剰が正で実現可能なら、残余辺は少なくとも1本ある。
                    auto best = std::numeric_limits<std::int64_t>::max();
                    for (const auto &edge : graph_[v])
                    {
                        if (edge.capacity > 0)
                        {
                            best = std::min(best, reduced_cost(v, edge));
                        }
                    }
                    potential_[v] = b_flow_subtract(potential_[v], b_flow_add(best, epsilon));
                    index = 0;
                    continue;
                }
                auto &edge = graph_[v][index];
                if (edge.capacity == 0 || reduced_cost(v, edge) >= 0)
                {
                    ++index;
                    continue;
                }
                const bool was_positive = excess_[edge.to] > 0;
                push(v, edge, std::min(excess_[v], edge.capacity));
                if (!was_positive && excess_[edge.to] > 0)
                {
                    active.push_back(edge.to);
                }
            }
        }
    }

    std::vector<std::vector<Arc>> graph_;
    std::vector<std::int64_t> potential_, excess_;
    std::vector<std::size_t> cursor_;
};
}

inline std::optional<BFlowResult> min_cost_b_flow(std::span<const std::int64_t> supply,
                                                   std::span<const BFlowEdge> edges)
{
    detail::check_b_flow_capacity(supply.size(), edges.size());
    const int n = static_cast<int>(supply.size());
    const int source = n, sink = n + 1;
    std::vector<std::int64_t> excess(supply.begin(), supply.end()), flow(edges.size());
    std::vector<int> ids(edges.size(), -1);
    mf_graph<std::int64_t> feasible(n + 2);
    feasible.reserve_edges(edges.size() + supply.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[from, to, lower, upper, cost] = edges[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n || lower > upper)
        {
            throw std::invalid_argument("min_cost_b_flow: invalid edge");
        }
#endif
        if (from == to)
        {
            flow[i] = cost < 0 ? upper : lower;
            continue;
        }
        flow[i] = lower;
        excess[from] = detail::b_flow_subtract(excess[from], lower);
        excess[to] = detail::b_flow_add(excess[to], lower);
        if (upper > lower)
        {
            ids[i] = feasible.add_edge(from, to, detail::b_flow_subtract(upper, lower));
        }
    }
    std::int64_t positive = 0, negative = 0;
    for (int v = 0; v < n; ++v)
    {
        if (excess[v] > 0)
        {
            positive = detail::b_flow_add(positive, excess[v]);
            feasible.add_edge(source, v, excess[v]);
        }
        else if (excess[v] < 0)
        {
            const auto amount = detail::b_flow_subtract(0, excess[v]);
            negative = detail::b_flow_add(negative, amount);
            feasible.add_edge(v, sink, amount);
        }
    }
    if (positive != negative || feasible.flow(source, sink, positive) != positive)
    {
        return std::nullopt;
    }

    // 費用をn+1倍する。換算費用が-1以上なら、長さn以下の閉路に負の整数費用はない。
    const std::int64_t factor = static_cast<std::int64_t>(n) + 1;
    const auto cost_limit = std::numeric_limits<std::int64_t>::max() / factor;
    std::int64_t epsilon = 0;
    detail::BFlowResidual residual(n);
    std::vector<std::size_t> positions(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (ids[i] == -1)
        {
            continue;
        }
        const auto &[from, to, lower, upper, cost] = edges[i];
        if (cost < -cost_limit || cost > cost_limit)
        {
            throw std::overflow_error("min_cost_b_flow: scaled edge cost out of range");
        }
        const auto scaled = cost * factor;
        const auto initial = feasible.get_edge(ids[i]);
        positions[i] = residual.add_edge(from, to, initial.cap, initial.flow, scaled);
        epsilon = std::max(epsilon, std::max(scaled, -scaled));
    }
    residual.optimize(epsilon);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (ids[i] != -1)
        {
            // lower + 残余の逆向き容量は元の上下限に収まる。
            flow[i] += residual.flow(edges[i].from, positions[i]);
        }
    }
    return BFlowResult{std::move(flow), residual.exact_potential(factor)};
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
