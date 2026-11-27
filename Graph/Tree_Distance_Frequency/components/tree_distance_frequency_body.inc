#ifndef SIRKIT_TREE_DISTANCE_FREQUENCY_DEFINED
#define SIRKIT_TREE_DISTANCE_FREQUENCY_DEFINED

namespace detail
{
using TreeDistanceNtt = NTT<998244353, 3>;
using TreeDistanceSecondNtt = NTT<469762049, 3>;

inline void check_tree_distance_frequency_size(std::size_t n)
{
    if (n > TreeDistanceNtt::max_length())
    {
        throw std::length_error("tree_distance_frequency: vertex count exceeds transform capacity");
    }
}

class TreeDistanceProducts
{
    TreeDistanceNtt first_;
    TreeDistanceSecondNtt second_;
    std::vector<std::uint32_t> work_;

public:
    void add(std::vector<std::int64_t> &frequency, const std::vector<std::uint32_t> &prefix,
             std::span<const std::uint32_t> branch, std::uint64_t coefficient_bound)
    {
        if (std::min(prefix.size(), branch.size()) <= 32)
        {
            for (std::size_t i = 0; i < prefix.size(); ++i)
            {
                for (std::size_t j = 0; j < branch.size(); ++j)
                {
                    frequency[i + j + 1] += std::int64_t{prefix[i]} * branch[j];
                }
            }
            return;
        }

        constexpr auto kFirstMod = TreeDistanceNtt::modulus();
        work_.assign(branch.begin(), branch.end());
        const auto first = first_.convolution(prefix, work_);
        if (coefficient_bound < kFirstMod)
        {
            for (std::size_t i = 0; i < first.size(); ++i)
            {
                frequency[i + 1] += first[i];
            }
            return;
        }

        // 頂点数の制限より、各係数は2つの法の積より小さい。
        constexpr auto kSecondMod = TreeDistanceSecondNtt::modulus();
        constexpr auto kInverse = ntt_power(kFirstMod % kSecondMod, kSecondMod - 2, kSecondMod);
        const auto second = second_.convolution(prefix, work_);
        for (std::size_t i = 0; i < first.size(); ++i)
        {
            const auto delta = (second[i] + kSecondMod - first[i] % kSecondMod) % kSecondMod;
            const auto quotient = std::uint64_t{delta} * kInverse % kSecondMod;
            frequency[i + 1] += static_cast<std::int64_t>(first[i] + quotient * kFirstMod);
        }
    }
};
} // namespace detail

inline std::vector<std::int64_t> tree_distance_frequency(const Graph &graph)
{
    detail::check_tree_distance_frequency_size(graph.size());
    const CentroidDecomposition decomposition(graph, false);
    std::vector<std::int64_t> frequency(graph.size());
    std::vector<unsigned char> removed(graph.size());
    std::vector<std::size_t> active_degree;
    active_degree.reserve(graph.size());
    for (const auto &neighbors : graph)
    {
        active_degree.push_back(neighbors.size());
    }

    struct Visit
    {
        int vertex;
        int parent;
        std::size_t depth;
    };
    struct Branch
    {
        std::size_t start;
        std::size_t width;
        std::size_t count;
        std::uint32_t largest;
    };

    detail::TreeDistanceProducts products;
    std::vector<Visit> visits;
    std::vector<Branch> branches;
    std::vector<std::uint32_t> coefficients, prefix;
    for (int centroid : decomposition.order())
    {
        removed[centroid] = 1;
        if (decomposition.component_size(centroid) == 1)
        {
            continue;
        }

        coefficients.clear();
        branches.clear();
        std::uint32_t leaves = 0;
        for (int root : graph[centroid])
        {
            if (removed[root])
            {
                continue;
            }
            --active_degree[root];
            if (active_degree[root] == 0)
            {
                ++leaves;
                continue;
            }
            const auto start = coefficients.size();
            visits.clear();
            visits.push_back({root, centroid, 0});
            for (std::size_t i = 0; i < visits.size(); ++i)
            {
                const auto [v, parent, depth] = visits[i];
                if (start + depth == coefficients.size())
                {
                    coefficients.push_back(0);
                }
                ++coefficients[start + depth];
                for (int to : graph[v])
                {
                    if (to != parent && !removed[to])
                    {
                        visits.push_back({to, v, depth + 1});
                    }
                }
            }
            const auto largest = *std::max_element(coefficients.begin() + start, coefficients.end());
            branches.push_back({start, coefficients.size() - start, visits.size(), largest});
        }

        prefix.assign(1, 1);
        frequency[1] += leaves;
        if (leaves != 0)
        {
            prefix.push_back(leaves);
            if (leaves > 1)
            {
                frequency[2] += std::int64_t{leaves} * (leaves - 1) / 2;
            }
        }

        // 短い枝から混ぜると、畳み込みの長さが現在の枝の深さに比例する。
        std::ranges::sort(branches, {}, &Branch::width);
        std::uint64_t prefix_count = std::uint64_t{leaves} + 1;
        std::uint32_t prefix_largest = std::max(std::uint32_t{1}, leaves);
        for (const auto &branch : branches)
        {
            const auto part = std::span<const std::uint32_t>(coefficients).subspan(branch.start, branch.width);
            const auto bound = std::min(std::uint64_t{prefix_largest} * branch.count, prefix_count * branch.largest);
            products.add(frequency, prefix, part, bound);

            prefix.resize(part.size() + 1);
            for (std::size_t i = 0; i < part.size(); ++i)
            {
                prefix[i + 1] += part[i];
                prefix_largest = std::max(prefix_largest, prefix[i + 1]);
            }
            prefix_count += branch.count;
        }
    }
    return frequency;
}

#endif
