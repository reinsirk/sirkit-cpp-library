#ifndef SIRKIT_BIPARTITE_MATCHING_ANALYSIS_DEFINED
#define SIRKIT_BIPARTITE_MATCHING_ANALYSIS_DEFINED

enum class BipartiteEdgeRole : std::uint8_t
{
    forbidden,
    optional,
    mandatory,
};

struct BipartiteMatchingAnalysis
{
    int matching_size = 0;
    std::vector<BipartiteEdgeRole> edge_role;
    std::vector<bool> left_can_be_free, right_can_be_free;
};

namespace detail
{
inline std::vector<bool> matching_reachability(
    const Graph &arcs, std::span<const int> matching, int offset)
{
    std::vector<bool> reached(arcs.size());
    std::vector<int> pending;
    pending.reserve(arcs.size());
    for (std::size_t i = 0; i < matching.size(); ++i)
    {
        if (matching[i] == -1)
        {
            const int vertex = offset + static_cast<int>(i);
            reached[vertex] = true;
            pending.push_back(vertex);
        }
    }
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        for (const int vertex : arcs[pending[i]])
        {
            if (!reached[vertex])
            {
                reached[vertex] = true;
                pending.push_back(vertex);
            }
        }
    }
    return reached;
}
} // namespace detail

inline BipartiteMatchingAnalysis analyze_bipartite_matching(
    int left, int right, std::span<const std::pair<int, int>> edges)
{
    if (left < 0 || right < 0)
    {
        throw std::invalid_argument("bipartite matching analysis: negative side size");
    }
    if (left > std::numeric_limits<int>::max() - right)
    {
        throw std::length_error("bipartite matching analysis: too many vertices");
    }
    const int n = left + right;
    BipartiteMatching matching(left, right);
    for (const auto &[u, v] : edges)
    {
        matching.add_edge(u, v);
    }
    BipartiteMatchingAnalysis result;
    result.matching_size = matching.solve();
    const auto left_matches = matching.left_matches();
    const auto right_matches = matching.right_matches();

    Graph arcs(n), reverse(n);
    scc_graph strongly_connected(n);
    strongly_connected.reserve_edges(edges.size());
    std::vector<bool> chosen(edges.size()), has_chosen(left);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, right_index] = edges[id];
        const int v = left + right_index;
        chosen[id] = left_matches[u] == right_index && !has_chosen[u];
        if (chosen[id])
        {
            has_chosen[u] = true;
        }
        const auto [from, to] = chosen[id] ? std::pair{v, u} : std::pair{u, v};
        arcs[from].push_back(to);
        reverse[to].push_back(from);
        strongly_connected.add_edge(from, to);
    }
    strongly_connected.build();
    const auto &component = strongly_connected.ids();
    const auto from_free_left = detail::matching_reachability(arcs, left_matches, 0);
    const auto to_free_right = detail::matching_reachability(reverse, right_matches, left);
    result.left_can_be_free.assign(from_free_left.begin(), from_free_left.begin() + left);
    result.right_can_be_free.assign(to_free_right.begin() + left, to_free_right.end());

    result.edge_role.reserve(edges.size());
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, right_index] = edges[id];
        const int v = left + right_index;
        const auto [from, to] = chosen[id] ? std::pair{v, u} : std::pair{u, v};
        // 最大マッチング間の差は、交互閉路か未対応頂点を移す交互路。
        const bool exchangeable = component[u] == component[v] || from_free_left[from] || to_free_right[to];
        result.edge_role.push_back(exchangeable ? BipartiteEdgeRole::optional
                                   : chosen[id] ? BipartiteEdgeRole::mandatory
                                                : BipartiteEdgeRole::forbidden);
    }
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
