#ifndef SIRKIT_BLOCK_CUT_FOREST_DEFINED
#define SIRKIT_BLOCK_CUT_FOREST_DEFINED

namespace detail
{
inline std::size_t block_cut_forest_size(int original, std::size_t blocks)
{
    if (blocks > static_cast<std::size_t>(std::numeric_limits<int>::max() - original))
    {
        throw std::length_error("block_cut_forest: too many block vertices");
    }
    return static_cast<std::size_t>(original) + blocks;
}
}

inline Graph block_cut_forest(int n, std::span<const std::pair<int, int>> edges)
{
    UndirectedLowLink low_link(n);
    low_link.reserve_edges(edges.size());
    for (const auto &[u, v] : edges)
    {
        low_link.add_edge(u, v);
    }
    if (low_link.edge_count() == 0)
    {
        return Graph(n);
    }
    low_link.build(true);
    auto blocks = low_link.biconnected_vertex_components(false);
    Graph forest(detail::block_cut_forest_size(n, blocks.size()));
    for (std::size_t i = 0; i < blocks.size(); ++i)
    {
        const int block = n + static_cast<int>(i);
        for (const int v : blocks[i])
        {
            forest[v].push_back(block);
        }
        forest[block] = std::move(blocks[i]);
    }
    return forest;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
