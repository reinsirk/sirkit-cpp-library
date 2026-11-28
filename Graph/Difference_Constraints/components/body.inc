#ifndef SIRKIT_DIFFERENCE_CONSTRAINTS_DEFINED
#define SIRKIT_DIFFERENCE_CONSTRAINTS_DEFINED

struct DifferenceConstraint
{
    int from, to;
    std::int64_t upper;
};

struct DifferenceConstraintBounds
{
    std::optional<std::int64_t> lower, upper;
};

struct DifferenceConstraintsResult
{
    bool feasible;
    std::vector<std::int64_t> values;
    std::vector<DifferenceConstraintBounds> bounds;
};

[[nodiscard]] inline DifferenceConstraintsResult solve_difference_constraints(
    int n, std::span<const DifferenceConstraint> constraints,
    int anchor = 0, std::int64_t inf = std::int64_t{1} << 60)
{
    if (n <= 0 || n == std::numeric_limits<int>::max())
    {
        throw std::length_error("solve_difference_constraints: vertex count");
    }
    if (anchor < 0 || anchor >= n)
    {
        throw std::out_of_range("solve_difference_constraints: anchor");
    }

    WGraph forward(static_cast<std::size_t>(n) + 1), backward(n);
    for (const auto &[u, v, limit] : constraints)
    {
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("solve_difference_constraints: constraint endpoint");
        }
        if (limit < -1000000000 || limit > 1000000000)
        {
            throw std::out_of_range("solve_difference_constraints: constraint weight");
        }
        forward[u].emplace_back(v, limit);
        backward[v].emplace_back(u, limit);
    }

    // 補助始点から全頂点へ辺を張ると、anchor に到達しない成分の矛盾も検出できる。
    forward[n].reserve(n);
    for (int v = 0; v < n; ++v)
    {
        forward[n].emplace_back(v, 0);
    }
    std::vector<std::int64_t> witness;
    if (BellmanFord(forward, witness, n, inf))
    {
        return {false, {}, {}};
    }
    forward.pop_back();
    witness.pop_back();
    const auto origin = witness[anchor];
    for (auto &value : witness)
    {
        value -= origin;
    }

    std::vector<std::int64_t> upper, negative_lower;
    (void)BellmanFord(forward, upper, anchor, inf);
    (void)BellmanFord(backward, negative_lower, anchor, inf);

    std::vector<DifferenceConstraintBounds> bounds(n);
    for (int v = 0; v < n; ++v)
    {
        if (upper[v] != inf)
        {
            bounds[v].upper = upper[v];
        }
        if (negative_lower[v] != inf)
        {
            bounds[v].lower = -negative_lower[v];
        }
    }
    return {true, std::move(witness), std::move(bounds)};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
