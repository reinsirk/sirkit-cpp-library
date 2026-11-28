// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tournament_Strong_Counts.cpp"

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

std::uint64_t power(std::uint64_t value, std::uint64_t exponent)
{
    std::uint64_t answer = 1;
    for (; exponent != 0; exponent /= 2)
    {
        if (exponent % 2 != 0)
        {
            answer = answer * value % kMod;
        }
        value = value * value % kMod;
    }
    return answer;
}

std::vector<std::uint32_t> exhaustive_sink_counts(int n)
{
    std::vector<std::pair<int, int>> edges;
    for (int to = 1; to < n; ++to)
    {
        for (int from = 0; from < to; ++from)
        {
            edges.emplace_back(from, to);
        }
    }
    std::vector<std::uint32_t> counts(n + 1);
    for (unsigned orientation = 0; orientation < (1U << edges.size()); ++orientation)
    {
        std::array<unsigned, 6> reach{};
        for (int v = 0; v < n; ++v)
        {
            reach[v] = 1U << v;
        }
        for (std::size_t e = 0; e < edges.size(); ++e)
        {
            auto [from, to] = edges[e];
            if (orientation >> e & 1U)
            {
                std::swap(from, to);
            }
            reach[from] |= 1U << to;
        }
        for (int intermediate = 0; intermediate < n; ++intermediate)
        {
            for (int v = 0; v < n; ++v)
            {
                if (reach[v] >> intermediate & 1U)
                {
                    reach[v] |= reach[intermediate];
                }
            }
        }
        int reachable = n;
        for (int v = 0; v < n; ++v)
        {
            reachable = std::min(reachable, std::popcount(reach[v]));
        }
        ++counts[reachable];
    }
    return counts;
}

std::vector<std::uint32_t> quadratic_counts(int n)
{
    std::vector<std::uint64_t> choose(n + 1);
    std::vector<std::uint64_t> all(n + 1);
    std::vector<std::uint32_t> strong(n + 1);
    choose[0] = 1;
    all[0] = 1;
    for (int size = 1; size <= n; ++size)
    {
        for (int k = size; k > 0; --k)
        {
            choose[k] = (choose[k] + choose[k - 1]) % kMod;
        }
        all[size] = power(2, size * (size - 1) / 2);
        std::uint64_t total = all[size];
        for (int k = 1; k < size; ++k)
        {
            const auto separated = choose[k] * strong[k] % kMod * all[size - k] % kMod;
            total = (total + kMod - separated) % kMod;
        }
        strong[size] = static_cast<std::uint32_t>(total);
    }
    return strong;
}

void check_expected_size(int n, std::uint32_t expected)
{
    const auto distribution = sirkit::tournament_sink_size_probabilities(n);
    std::uint64_t average = 0;
    for (std::size_t k = 1; k < distribution.size(); ++k)
    {
        average = (average + k * distribution[k]) % kMod;
    }
    const auto sum = std::accumulate(distribution.begin(), distribution.end(), std::uint64_t{0});
    assert(sum % kMod == 1);
    assert(average == expected);
}
} // namespace

int main()
{
    const auto strong = sirkit::tournament_strong_counts(6);
    for (int n = 0; n <= 6; ++n)
    {
        const auto counts = exhaustive_sink_counts(n);
        const auto total = std::accumulate(counts.begin(), counts.end(), 0U);
        const auto inverse = power(total, kMod - 2);
        const auto distribution = sirkit::tournament_sink_size_probabilities(n);
        assert(distribution.size() == counts.size());
        assert(strong[n] == (n == 0 ? 0 : counts[n]));
        for (int k = 0; k <= n; ++k)
        {
            assert(distribution[k] == counts[k] * inverse % kMod);
        }
    }
    assert(sirkit::tournament_strong_counts(0) == std::vector<std::uint32_t>{0});
    assert(sirkit::tournament_strong_counts(110) == quadratic_counts(110));

    for (const int invalid : {-1, 1 << 22})
    {
        bool caught = false;
        try
        {
            (void)sirkit::tournament_strong_counts(invalid);
        }
        catch (const std::length_error &)
        {
            caught = true;
        }
        assert(caught);
    }
    check_expected_size(100, 550420818);
    check_expected_size(2026, 926770690);
    check_expected_size(200000, 945697629);
}
