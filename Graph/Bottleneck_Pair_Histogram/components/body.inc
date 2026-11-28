#ifndef SIRKIT_BOTTLENECK_PAIR_HISTOGRAM_DEFINED
#define SIRKIT_BOTTLENECK_PAIR_HISTOGRAM_DEFINED

struct BottleneckEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct BottleneckPairBucket
{
    std::int64_t weight;
    std::uint64_t minimax_pairs = 0;
    std::uint64_t maximin_pairs = 0;
};

inline std::vector<BottleneckPairBucket> bottleneck_pair_histogram(
    std::size_t n, std::span<const BottleneckEdge> edges)
{
    UnionFind components(n);
    std::vector<BottleneckEdge> sorted(edges.begin(), edges.end());
    std::ranges::sort(sorted, {}, &BottleneckEdge::weight);

    std::vector<BottleneckPairBucket> result;
    std::vector<std::size_t> boundary{0};
    for (std::size_t i = 0; i < sorted.size();)
    {
        const auto weight = sorted[i].weight;
        result.push_back({weight});
        do
        {
            ++i;
        } while (i < sorted.size() && sorted[i].weight == weight);
        boundary.push_back(i);
    }

    auto merge_group = [&](std::size_t group)
    {
        std::uint64_t added = 0;
        for (std::size_t i = boundary[group]; i < boundary[group + 1]; ++i)
        {
            const int u = components.root(sorted[i].from);
            const int v = components.root(sorted[i].to);
            if (u != v)
            {
                added += static_cast<std::uint64_t>(components.size(u)) * components.size(v);
                components.merge(u, v);
            }
        }
        return added;
    };
    for (std::size_t group = 0; group < result.size(); ++group)
    {
        result[group].minimax_pairs = merge_group(group);
    }
    components.reset();
    for (std::size_t group = result.size(); group != 0;)
    {
        --group;
        result[group].maximin_pairs = merge_group(group);
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
