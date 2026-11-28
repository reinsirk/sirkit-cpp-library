#ifndef SIRKIT_AFFINE_WALK_RANGES_DEFINED
#define SIRKIT_AFFINE_WALK_RANGES_DEFINED

struct AffineWalkEdge
{
    int from, to;
    bool reflect;
    std::int32_t shift;
};

[[nodiscard]] inline std::vector<WalkWeightRange> affine_walk_ranges(
    int n, std::span<const AffineWalkEdge> edges, int source)
{
    if (n <= 0 || n > std::numeric_limits<int>::max() / 2)
    {
        throw std::length_error("affine_walk_ranges: vertex count");
    }
    if (source < 0 || source >= n)
    {
        throw std::out_of_range("affine_walk_ranges: source");
    }
    std::vector<WalkWeightEdge> lifted;
    lifted.reserve(2 * edges.size());
    for (const auto &[from, to, reflect, shift] : edges)
    {
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("affine_walk_ranges: edge endpoint");
        }
        if (shift == std::numeric_limits<std::int32_t>::min())
        {
            throw std::out_of_range("affine_walk_ranges: shift");
        }
        const int change = static_cast<int>(reflect);
        lifted.push_back({2 * from, 2 * to + change, reflect ? -shift : shift});
        lifted.push_back({2 * from + 1, 2 * to + (1 ^ change), reflect ? shift : -shift});
    }

    auto ranges = walk_weight_ranges(2 * n, lifted, 2 * source);
    std::vector<WalkWeightRange> answer(n);
    for (int vertex = 0; vertex < n; ++vertex)
    {
        auto &range = answer[vertex];
        for (int sign = 0; sign < 2; ++sign)
        {
            const auto &part = ranges[2 * vertex + sign];
            if (!part.reachable)
            {
                continue;
            }
            if (part.unbounded)
            {
                range = {true, true, 0, 0};
                break;
            }
            const auto low = sign == 0 ? part.minimum : -part.maximum;
            const auto high = sign == 0 ? part.maximum : -part.minimum;
            if (!range.reachable)
            {
                range = {true, false, low, high};
            }
            else
            {
                range.minimum = std::min(range.minimum, low);
                range.maximum = std::max(range.maximum, high);
            }
        }
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
