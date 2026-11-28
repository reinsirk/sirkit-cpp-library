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
