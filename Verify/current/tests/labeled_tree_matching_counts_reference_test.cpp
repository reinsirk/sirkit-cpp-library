// SPDX-License-Identifier: CC0-1.0
#include "Math/Labeled_Tree_Matching_Counts.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace
{
constexpr auto kMod = std::uint64_t{998244353};

std::uint64_t power(std::uint64_t base, std::size_t exponent)
{
    std::uint64_t result = 1;
    for (std::size_t bits = exponent; bits; bits >>= 1)
    {
        if (bits & 1)
        {
            result = result * base % kMod;
        }
        base = base * base % kMod;
    }
    return result;
}

bool winning(const std::vector<std::vector<int>> &graph, int at, int parent)
{
    for (const int to : graph[at])
    {
        if (to != parent && !winning(graph, to, at))
        {
            return true;
        }
    }
    return false;
}

bool match_all(const std::vector<std::vector<int>> &graph, unsigned mask)
{
    if (mask == 0)
    {
        return true;
    }
    unsigned vertex = 0;
    while ((mask >> vertex & 1) == 0)
    {
        ++vertex;
    }
    mask ^= 1U << vertex;
    for (const int other : graph[vertex])
    {
        if ((mask >> other & 1) && match_all(graph, mask ^ (1U << other)))
        {
            return true;
        }
    }
    return false;
}

std::pair<std::uint32_t, std::uint32_t> brute_trees(int n)
{
    std::vector<int> code(static_cast<std::size_t>(n - 2));
    std::uint32_t losing = 0, perfect = 0;
    for (std::uint64_t id = 0; id < power(n, n - 2); ++id)
    {
        auto encoded = id;
        std::vector<int> degree(n, 1);
        for (int &v : code)
        {
            v = static_cast<int>(encoded % n);
            encoded /= n;
            ++degree[v];
        }
        std::vector<std::vector<int>> graph(n);
        const auto connect = [&](int u, int v)
        {
            graph[u].push_back(v);
            graph[v].push_back(u);
            --degree[u];
            --degree[v];
        };
        for (const int other : code)
        {
            connect(static_cast<int>(std::find(degree.begin(), degree.end(), 1) - degree.begin()), other);
        }
        const auto first = std::find(degree.begin(), degree.end(), 1);
        const auto second = std::find(first + 1, degree.end(), 1);
        connect(static_cast<int>(first - degree.begin()), static_cast<int>(second - degree.begin()));
        losing += !winning(graph, 0, -1);
        perfect += match_all(graph, (1U << n) - 1);
    }
    return {losing, perfect};
}
}

int main()
{
    using sirkit::labeled_tree_exposable_root_counts;
    using sirkit::labeled_tree_perfect_matching_counts;
    assert(labeled_tree_exposable_root_counts(0) == std::vector<std::uint32_t>{0});
    assert(labeled_tree_perfect_matching_counts(0) == std::vector<std::uint32_t>{0});
    const auto roots = labeled_tree_exposable_root_counts(7), perfect = labeled_tree_perfect_matching_counts(7);
    assert(roots[1] == 1 && perfect[1] == 0);
    for (int n = 2; n <= 7; ++n)
    {
        const auto [losing, allowed] = brute_trees(n);
        assert(roots[n] == losing && perfect[n] == allowed);
    }

    // Compute the coefficients independently, from the derivative of exp(T-L).
    constexpr std::size_t slow_limit = 2048;
    const auto checked = labeled_tree_exposable_root_counts(slow_limit);
    std::vector<std::uint64_t> tree(slow_limit + 1), losing(slow_limit + 1);
    losing[1] = 1;
    std::uint64_t inverse_factorial = 1, factorial = 1;
    for (std::size_t i = 1; i <= slow_limit; ++i)
    {
        inverse_factorial = inverse_factorial * power(i, kMod - 2) % kMod;
        tree[i] = power(i, i - 1) * inverse_factorial % kMod;
        if (i > 1)
        {
            std::uint64_t sum = 0;
            for (std::size_t j = 1; j < i; ++j)
            {
                const auto derivative = (tree[j] + kMod - losing[j]) * j % kMod;
                sum = (sum + derivative * losing[i - j]) % kMod;
            }
            losing[i] = sum * power(i - 1, kMod - 2) % kMod;
        }
        assert(checked[i] == losing[i] * factorial % kMod);
        factorial = factorial * i % kMod;
    }

    // Check the full-size result as a formal equation, coefficient by coefficient.
    constexpr std::size_t maximum = 250000;
    const auto large = labeled_tree_exposable_root_counts(maximum);
    std::vector<std::uint32_t> normalized(maximum), shifted(maximum);
    inverse_factorial = 1;
    for (std::size_t i = 0; i < maximum; ++i)
    {
        normalized[i] = large[i + 1] * inverse_factorial % kMod;
        inverse_factorial = inverse_factorial * power(i + 1, kMod - 2) % kMod;
    }
    std::copy(normalized.begin(), normalized.end() - 1, shifted.begin() + 1);
    const auto actual = sirkit::FormalPowerSeries(std::move(normalized)).log(maximum) +
                        sirkit::FormalPowerSeries(std::move(shifted));
    assert(actual[0] == 0);
    inverse_factorial = 1;
    for (std::size_t i = 1; i < maximum; ++i)
    {
        inverse_factorial = inverse_factorial * power(i, kMod - 2) % kMod;
        assert(actual[i] == power(i, i - 1) * inverse_factorial % kMod);
    }

    try
    {
        (void)labeled_tree_exposable_root_counts(sirkit::FormalPowerSeries::max_precision() + 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
