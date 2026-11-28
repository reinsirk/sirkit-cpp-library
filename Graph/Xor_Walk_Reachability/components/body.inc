#ifndef SIRKIT_XOR_WALK_REACHABILITY_DEFINED
#define SIRKIT_XOR_WALK_REACHABILITY_DEFINED

namespace detail
{
inline std::size_t xor_walk_state_width(unsigned width, std::size_t n, std::size_t limit)
{
    if (width >= std::numeric_limits<std::size_t>::digits)
    {
        throw std::length_error("XOR walk: state width is too large");
    }
    const auto states = std::size_t{1} << width;
    if (states > limit / n)
    {
        throw std::length_error("XOR walk: too many states");
    }
    return states;
}
} // namespace detail

class XorWalkReachability
{
public:
    XorWalkReachability(int n, std::span<const XorWalkEdge> edges, int source, bool directed = true)
    {
        detail::check_xor_walk_vertex(source, n);
        std::uint64_t all_bits = 0;
        for (const auto &edge : edges)
        {
            detail::check_xor_walk_vertex(edge.from, n);
            detail::check_xor_walk_vertex(edge.to, n);
            all_bits |= edge.weight;
        }
        const auto width = std::bit_width(all_bits);
        states_ = detail::xor_walk_state_width(width, n, visited_.max_size());
        visited_.resize(n * states_);
        minimum_.assign(n, states_);
        maximum_.resize(n);

        std::vector<std::vector<std::pair<int, std::size_t>>> adjacent(n);
        for (const auto &edge : edges)
        {
            adjacent[edge.from].emplace_back(edge.to, edge.weight);
            if (!directed && edge.from != edge.to)
            {
                adjacent[edge.to].emplace_back(edge.from, edge.weight);
            }
        }

        const auto start = static_cast<std::size_t>(source) << width;
        std::vector<std::size_t> pending{start};
        visited_[start] = true;
        while (!pending.empty())
        {
            const auto state = pending.back();
            pending.pop_back();
            const auto u = state >> width;
            const auto value = state & (states_ - 1);
            minimum_[u] = std::min(minimum_[u], value);
            maximum_[u] = std::max(maximum_[u], value);
            for (const auto &[v, weight] : adjacent[u])
            {
                const auto next = (static_cast<std::size_t>(v) << width) | (value ^ weight);
                if (!visited_[next])
                {
                    visited_[next] = true;
                    pending.push_back(next);
                }
            }
        }
    }

    [[nodiscard]] bool reachable(int vertex, std::uint64_t value) const
    {
        check_vertex(vertex);
        return value < states_ && visited_[vertex * states_ + static_cast<std::size_t>(value)];
    }

    [[nodiscard]] std::optional<std::uint64_t> min_xor(int vertex) const
    {
        check_vertex(vertex);
        if (minimum_[vertex] == states_)
        {
            return std::nullopt;
        }
        return minimum_[vertex];
    }

    [[nodiscard]] std::optional<std::uint64_t> max_xor(int vertex) const
    {
        check_vertex(vertex);
        if (minimum_[vertex] == states_)
        {
            return std::nullopt;
        }
        return maximum_[vertex];
    }

private:
    void check_vertex(int vertex) const
    {
        detail::check_xor_walk_vertex(vertex, static_cast<int>(minimum_.size()));
    }

    std::size_t states_ = 0;
    std::vector<bool> visited_;
    std::vector<std::size_t> minimum_, maximum_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
