#ifndef SIRKIT_WALK_WEIGHT_CONGRUENCE_DEFINED
#define SIRKIT_WALK_WEIGHT_CONGRUENCE_DEFINED

struct WalkCongruenceEdge
{
    int from;
    int to;
    std::int32_t weight;
};

struct WalkWeightClass
{
    std::int64_t offset;
    std::uint64_t period;

    [[nodiscard]] bool matches(std::int64_t residue, std::uint64_t modulus) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (modulus == 0)
        {
            throw std::invalid_argument("WalkWeightClass: zero modulus");
        }
#endif
        const auto divisor = std::gcd(modulus, period);
        auto normalize = [divisor](std::int64_t x)
        {
            const auto bits = static_cast<std::uint64_t>(x);
            if (x >= 0)
            {
                return bits % divisor;
            }
            const auto magnitude = (0 - bits) % divisor;
            return magnitude == 0 ? 0 : divisor - magnitude;
        };
        return normalize(residue) == normalize(offset);
    }
};

class WalkWeightCongruence
{
public:
    WalkWeightCongruence(int n, std::span<const WalkCongruenceEdge> edges)
    {
        scc_graph components(n);
        components.reserve_edges(edges.size());
        for (const auto &[u, v, weight] : edges)
        {
            components.add_edge(u, v);
        }
        periods_.resize(components.build());
        component_ = components.ids();

        struct Arc
        {
            int to;
            std::int32_t weight;
        };
        std::vector<std::vector<Arc>> graph(n);
        for (const auto &[u, v, weight] : edges)
        {
            if (component_[u] == component_[v])
            {
                graph[u].push_back({v, weight});
            }
        }

        // 単純パスは高々 n-1 辺。int32_t 重みの総和と2本のパスの差は int64_t に収まる。
        constexpr auto kUnvisited = std::numeric_limits<std::int64_t>::max();
        potential_.assign(n, kUnvisited);
        std::vector<int> stack;
        for (int root = 0; root < n; ++root)
        {
            if (potential_[root] != kUnvisited)
            {
                continue;
            }
            potential_[root] = 0;
            stack.push_back(root);
            auto &period = periods_[component_[root]];
            while (!stack.empty())
            {
                const int u = stack.back();
                stack.pop_back();
                for (const auto &[v, weight] : graph[u])
                {
                    const auto candidate = potential_[u] + weight;
                    if (potential_[v] == kUnvisited)
                    {
                        potential_[v] = candidate;
                        stack.push_back(v);
                    }
                    else
                    {
                        const auto difference = candidate - potential_[v];
                        const auto magnitude = difference < 0 ? -difference : difference;
                        period = std::gcd(period, static_cast<std::uint64_t>(magnitude));
                    }
                }
            }
        }
    }

    [[nodiscard]] std::uint64_t period(int vertex) const
    {
        check_vertex(vertex);
        return periods_[component_[vertex]];
    }

    [[nodiscard]] std::span<const std::uint64_t> periods() const noexcept
    {
        return periods_;
    }

    [[nodiscard]] std::optional<WalkWeightClass> relation(int source, int target) const
    {
        check_vertex(source);
        check_vertex(target);
        if (component_[source] != component_[target])
        {
            return std::nullopt;
        }
        return WalkWeightClass{potential_[target] - potential_[source], periods_[component_[source]]};
    }

private:
    void check_vertex([[maybe_unused]] int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(vertex) >= component_.size())
        {
            throw std::out_of_range("WalkWeightCongruence: invalid vertex");
        }
#endif
    }

    std::vector<int> component_;
    std::vector<std::int64_t> potential_;
    std::vector<std::uint64_t> periods_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
