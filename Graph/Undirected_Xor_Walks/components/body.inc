#ifndef SIRKIT_XOR_WALK_EDGE_DEFINED
#define SIRKIT_XOR_WALK_EDGE_DEFINED

struct XorWalkEdge
{
    int from, to;
    std::uint64_t weight;
};

namespace detail
{
inline void check_xor_walk_vertex(int vertex, int n)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (vertex < 0 || vertex >= n)
    {
        throw std::out_of_range("XOR walk: vertex out of range");
    }
#else
    (void)vertex;
    (void)n;
#endif
}
} // namespace detail

#endif

#ifndef SIRKIT_UNDIRECTED_XOR_WALKS_DEFINED
#define SIRKIT_UNDIRECTED_XOR_WALKS_DEFINED

class UndirectedXorWalks
{
public:
    UndirectedXorWalks(int n, std::span<const XorWalkEdge> edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0)
        {
            throw std::invalid_argument("undirected XOR walks: negative vertex count");
        }
#endif
        std::vector<std::vector<std::size_t>> adjacent(n);
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            const auto &[u, v, weight] = edges[id];
            detail::check_xor_walk_vertex(u, n);
            detail::check_xor_walk_vertex(v, n);
            adjacent[u].push_back(id);
            if (u != v)
            {
                adjacent[v].push_back(id);
            }
        }

        component_.assign(n, -1);
        reduced_.resize(n);
        vertices_.reserve(n);
        for (int vertex = 0; vertex < n; ++vertex)
        {
            if (component_[vertex] != -1)
            {
                continue;
            }
            const int component = static_cast<int>(basis_index_.size());
            const auto begin = vertices_.size();
            vertices_.push_back(vertex);
            component_[vertex] = component;
            for (auto i = begin; i < vertices_.size(); ++i)
            {
                const int u = vertices_[i];
                for (const auto id : adjacent[u])
                {
                    const auto &edge = edges[id];
                    const int v = edge.from == u ? edge.to : edge.from;
                    if (component_[v] == -1)
                    {
                        component_[v] = component;
                        reduced_[v] = reduced_[u] ^ edge.weight;
                        vertices_.push_back(v);
                    }
                }
            }

            // 全域木を使って辺ごとに閉じた歩道のXORを作る。
            XorBasis basis;
            for (auto i = begin; i < vertices_.size(); ++i)
            {
                const int u = vertices_[i];
                for (const auto id : adjacent[u])
                {
                    const auto &edge = edges[id];
                    if (edge.from == u)
                    {
                        const auto cycle = reduced_[u] ^ edge.weight ^ reduced_[edge.to];
                        if (cycle != 0)
                        {
                            basis.insert(cycle);
                        }
                    }
                }
            }
            int index = -1;
            if (basis.rank() != 0)
            {
                index = static_cast<int>(bases_.size());
                bases_.push_back(basis);
                // 基底のピボットを消す写像はXORに関して線形。
                for (auto i = begin; i < vertices_.size(); ++i)
                {
                    const int u = vertices_[i];
                    reduced_[u] = basis.min_xor(reduced_[u]);
                }
            }
            basis_index_.push_back(index);
            offsets_.push_back(vertices_.size());
        }
    }

    [[nodiscard]] std::optional<std::uint64_t> min_xor(int u, int v) const
    {
        if (!connected(u, v))
        {
            return std::nullopt;
        }
        return reduced_[u] ^ reduced_[v];
    }

    [[nodiscard]] std::optional<std::uint64_t> max_xor(int u, int v) const
    {
        const auto base = min_xor(u, v);
        if (!base)
        {
            return std::nullopt;
        }
        const int index = basis_index_[component_[u]];
        return index < 0 ? *base : bases_[index].max_xor(*base);
    }

    [[nodiscard]] bool attainable(int u, int v, std::uint64_t value) const
    {
        const auto base = min_xor(u, v);
        if (!base)
        {
            return false;
        }
        const int index = basis_index_[component_[u]];
        return index < 0 ? *base == value : bases_[index].contains(*base ^ value);
    }

    [[nodiscard]] std::uint64_t count_pairs_at_most(std::uint64_t bound) const
    {
        std::uint64_t count = 0;
        for (std::size_t c = 0; c < basis_index_.size(); ++c)
        {
            const auto begin = offsets_[c];
            const std::uint64_t length = offsets_[c + 1] - begin;
            if (length < 2)
            {
                continue;
            }
            if (bound == std::numeric_limits<std::uint64_t>::max())
            {
                count += length * (length - 1) / 2;
                continue;
            }
            std::vector<std::uint64_t> values;
            values.reserve(length);
            for (auto i = begin; i < offsets_[c + 1]; ++i)
            {
                values.push_back(reduced_[vertices_[i]]);
            }
            count += PairXorOrder<>(std::move(values)).count_less(bound + 1);
        }
        return count;
    }

private:
    [[nodiscard]] bool connected(int u, int v) const
    {
        const auto n = static_cast<int>(component_.size());
        detail::check_xor_walk_vertex(u, n);
        detail::check_xor_walk_vertex(v, n);
        return component_[u] == component_[v];
    }

    std::vector<int> component_, basis_index_, vertices_;
    std::vector<std::size_t> offsets_{0};
    std::vector<std::uint64_t> reduced_;
    std::vector<XorBasis> bases_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
