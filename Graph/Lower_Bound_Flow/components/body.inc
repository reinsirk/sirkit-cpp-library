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
