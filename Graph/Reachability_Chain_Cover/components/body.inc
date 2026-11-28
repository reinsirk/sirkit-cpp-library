#ifndef SIRKIT_REACHABILITY_CHAIN_COVER_DEFINED
#define SIRKIT_REACHABILITY_CHAIN_COVER_DEFINED

namespace detail
{
inline void check_chain_cover_network_size(std::size_t components)
{
    constexpr auto kMaximum = static_cast<std::size_t>((std::numeric_limits<int>::max() - 2) / 2);
    if (components > kMaximum)
    {
        throw std::length_error("reachability chain cover: too many components");
    }
}
} // namespace detail

inline std::vector<std::vector<int>> minimum_reachability_chain_cover(
    int n, std::span<const std::pair<int, int>> edges)
{
    scc_graph graph(n);
    graph.reserve_edges(edges.size());
    for (const auto &[u, v] : edges)
    {
        graph.add_edge(u, v);
    }
    const auto groups = graph.scc();
    detail::check_chain_cover_network_size(groups.size());
    const int count = static_cast<int>(groups.size());
    const auto dag = graph.condensation();

    // 各成分だけを通る流量1の歩道から始め、残余グラフで歩道同士をつなぐ。
    const int cancel_from = 2 * count, cancel_to = cancel_from + 1;
    mf_graph<int> residual(cancel_to + 1);
    std::vector<int> cancel_starts(count), cancel_ends(count);
    struct Transit
    {
        int to, id, remaining = 0;
    };
    std::vector<std::vector<Transit>> outgoing(count);
    for (int v = 0; v < count; ++v)
    {
        cancel_starts[v] = residual.add_edge(2 * v, cancel_to, 1);
        cancel_ends[v] = residual.add_edge(cancel_from, 2 * v + 1, 1);
        residual.add_edge(2 * v, 2 * v + 1, count);
        outgoing[v].reserve(dag[v].size());
        for (const int u : dag[v])
        {
            const int flow_edge = residual.add_edge(2 * v + 1, 2 * u, count);
            outgoing[v].push_back({u, flow_edge});
        }
    }
    const int saved = residual.flow(cancel_from, cancel_to);
    std::vector<bool> can_end(count), assigned(count);
    for (int v = 0; v < count; ++v)
    {
        can_end[v] = residual.get_edge(cancel_ends[v]).flow == 0;
        for (auto &transit : outgoing[v])
        {
            transit.remaining = residual.get_edge(transit.id).flow;
        }
    }

    std::vector<std::vector<int>> chains;
    chains.reserve(count - saved);
    std::vector<std::size_t> cursor(count);
    for (int start = 0; start < count; ++start)
    {
        if (residual.get_edge(cancel_starts[start]).flow != 0)
        {
            continue;
        }
        auto &chain = chains.emplace_back();
        int v = start;
        for (;;)
        {
            if (!assigned[v])
            {
                chain.insert(chain.end(), groups[v].begin(), groups[v].end());
                assigned[v] = true;
            }
            if (can_end[v])
            {
                can_end[v] = false;
                break;
            }
            while (outgoing[v][cursor[v]].remaining == 0)
            {
                ++cursor[v];
            }
            auto &next = outgoing[v][cursor[v]];
            --next.remaining;
            v = next.to;
        }
    }
    return chains;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
