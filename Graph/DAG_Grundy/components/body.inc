#ifndef SIRKIT_DAG_GRUNDY_DEFINED
#define SIRKIT_DAG_GRUNDY_DEFINED

inline std::optional<std::vector<int>> dag_grundy_numbers(const Graph &moves)
{
    const auto order = TopologicalSort(moves);
    if (order.size() != moves.size())
    {
        return std::nullopt;
    }

    std::vector<int> grundy(moves.size()), marked(moves.size() + 1, -1);
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int vertex = *it;
        for (const int next : moves[vertex])
        {
            marked[grundy[next]] = vertex;
        }
        int mex = 0;
        while (marked[mex] == vertex)
        {
            ++mex;
        }
        grundy[vertex] = mex;
    }
    return grundy;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
