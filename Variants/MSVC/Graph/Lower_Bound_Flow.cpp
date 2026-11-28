// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Lower_Bound_Flow.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lower_bound_flow.md
//
// API一覧
// LowerBoundFlow<Cap=std::int64_t>(n): n頂点の有向グラフ。頂点0..n-1。
// add_edge(from,to,lower,upper): 0<=lower<=upperの辺を追加し、入力順の辺番号を返す。償却O(1)。
// size()/edge_count(): 頂点数/辺数。O(1)。
// circulation(): 全頂点で流量を保存する流れ。入力順の辺流量を返す。
// min_flow(s,t,limit=Capの上限)/max_flow(s,t,limit): [0,limit]で最小/最大のs-t流。
//   Flow{value, edge_flow}を返す。sとtは異なる頂点。解がなければnullopt。
//   各頂点の入出下限和（自己辺を除く）と必要流量の合計はCapで表せること。
//   解を求める呼び出しは最大流を高々2回とO(n+m)の追加処理。領域O(n+m)。辺を保持したまま再計算できる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <optional>
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

#ifndef SIRKIT_LOWER_BOUND_FLOW_DEFINED
#define SIRKIT_LOWER_BOUND_FLOW_DEFINED

namespace detail
{
inline int lower_bound_flow_vertices(int n)
{
    if (n < 0 || n > std::numeric_limits<int>::max() - 2)
    {
        throw std::length_error("LowerBoundFlow: invalid vertex count");
    }
    return n;
}

template <class Cap>
Cap lower_bound_flow_sum(Cap a, Cap b)
{
    if (a > std::numeric_limits<Cap>::max() - b)
    {
        throw std::overflow_error("LowerBoundFlow: demand exceeds capacity type");
    }
    return static_cast<Cap>(a + b);
}

template <class Cap>
void check_lower_bound_flow_nonnegative(Cap value)
{
#ifndef SKIP_RUNTIME_CHECKS
    if constexpr (std::is_signed_v<Cap>)
    {
        if (value < 0)
        {
            throw std::invalid_argument("LowerBoundFlow: negative capacity");
        }
    }
#endif
    static_cast<void>(value);
}
} // namespace detail

template <class Cap = std::int64_t>
class LowerBoundFlow
{
    static_assert(std::is_integral_v<Cap> && !std::is_same_v<Cap, bool>);

    struct Edge
    {
        int from, to;
        Cap lower, upper;
    };

    struct Terminals
    {
        int source, target;
        Cap limit;
    };

    struct Residual
    {
        mf_graph<Cap> graph;
        Cap value;
    };

public:
    struct Flow
    {
        Cap value;
        std::vector<Cap> edge_flow;
    };

    explicit LowerBoundFlow(int n) : n_(detail::lower_bound_flow_vertices(n))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return n_;
    }

    [[nodiscard]] std::size_t edge_count() const noexcept
    {
        return edges_.size();
    }

    std::size_t add_edge(int from, int to, Cap lower, Cap upper)
    {
        check_vertex(from);
        check_vertex(to);
        detail::check_lower_bound_flow_nonnegative(lower);
#ifndef SKIP_RUNTIME_CHECKS
        if (upper < lower)
        {
            throw std::invalid_argument("LowerBoundFlow: upper capacity below lower");
        }
#endif
        edges_.push_back({from, to, lower, upper});
        return edges_.size() - 1;
    }

    [[nodiscard]] std::optional<std::vector<Cap>> circulation() const
    {
        const auto residual = prepare(std::nullopt);
        if (!residual)
        {
            return std::nullopt;
        }
        return extract(residual->graph);
    }

    [[nodiscard]] std::optional<Flow> min_flow(int s, int t, Cap limit = std::numeric_limits<Cap>::max()) const
    {
        return extremum(s, t, limit, false);
    }

    [[nodiscard]] std::optional<Flow> max_flow(int s, int t, Cap limit = std::numeric_limits<Cap>::max()) const
    {
        return extremum(s, t, limit, true);
    }

private:
    void check_vertex(int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || v >= n_)
        {
            throw std::out_of_range("LowerBoundFlow: invalid vertex");
        }
#else
        static_cast<void>(v);
#endif
    }

    [[nodiscard]] std::optional<Residual> prepare(std::optional<Terminals> terminals) const
    {
        const int super_source = n_;
        const int super_sink = n_ + 1;
        mf_graph<Cap> graph(n_ + 2);
        graph.reserve_edges(edges_.size());
        std::vector<Cap> incoming(n_), outgoing(n_);
        for (const auto &[from, to, lower, upper] : edges_)
        {
            if (from != to)
            {
                incoming[to] = detail::lower_bound_flow_sum(incoming[to], lower);
                outgoing[from] = detail::lower_bound_flow_sum(outgoing[from], lower);
            }
            graph.add_edge(from, to, static_cast<Cap>(upper - lower));
        }
        int return_edge = -1;
        if (terminals)
        {
            return_edge = graph.add_edge(terminals->target, terminals->source, terminals->limit);
        }

        Cap demand{};
        for (int v = 0; v < n_; ++v)
        {
            if (incoming[v] > outgoing[v])
            {
                const auto delta = static_cast<Cap>(incoming[v] - outgoing[v]);
                demand = detail::lower_bound_flow_sum(demand, delta);
                graph.add_edge(super_source, v, delta);
            }
            else if (incoming[v] < outgoing[v])
            {
                graph.add_edge(v, super_sink, static_cast<Cap>(outgoing[v] - incoming[v]));
            }
        }
        if (graph.flow(super_source, super_sink, demand) != demand)
        {
            return std::nullopt;
        }
        const Cap base = terminals ? graph.get_edge(return_edge).flow : Cap{};
        for (std::size_t id = edges_.size(); id < graph.edge_count(); ++id)
        {
            graph.change_edge(static_cast<int>(id), Cap{}, Cap{});
        }
        return Residual{std::move(graph), base};
    }

    [[nodiscard]] std::vector<Cap> extract(const mf_graph<Cap> &graph) const
    {
        std::vector<Cap> flow;
        flow.reserve(edges_.size());
        for (std::size_t id = 0; id < edges_.size(); ++id)
        {
            flow.push_back(static_cast<Cap>(edges_[id].lower + graph.get_edge(static_cast<int>(id)).flow));
        }
        return flow;
    }

    [[nodiscard]] std::optional<Flow> extremum(int s, int t, Cap limit, bool maximum) const
    {
        check_vertex(s);
        check_vertex(t);
        detail::check_lower_bound_flow_nonnegative(limit);
#ifndef SKIP_RUNTIME_CHECKS
        if (s == t)
        {
            throw std::invalid_argument("LowerBoundFlow: source equals sink");
        }
#endif
        auto residual = prepare(Terminals{s, t, limit});
        if (!residual)
        {
            return std::nullopt;
        }
        auto &graph = residual->graph;
        const Cap base = residual->value;
        const Cap value = maximum
                              ? static_cast<Cap>(base + graph.flow(s, t, static_cast<Cap>(limit - base)))
                              : static_cast<Cap>(base - graph.flow(t, s, base));
        return Flow{value, extract(graph)};
    }

    int n_;
    std::vector<Edge> edges_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
