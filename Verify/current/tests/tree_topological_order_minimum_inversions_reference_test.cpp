// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Topological_Order_Minimum_Inversions.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Weight = std::uint64_t;
using sirkit::minimum_inversion_topological_order;

Weight slow(const std::vector<int> &parent, const std::vector<Weight> &c, const std::vector<Weight> &d,
            unsigned done = 0, Weight d_prefix = 0)
{
    const int n = static_cast<int>(parent.size());
    if (done == (1U << n) - 1)
    {
        return 0;
    }
    auto best = std::numeric_limits<Weight>::max();
    for (int v = 0; v < n; ++v)
    {
        if (((done >> v) & 1) == 0 && (parent[v] < 0 || ((done >> parent[v]) & 1) != 0))
        {
            const auto next = c[v] * d_prefix + slow(parent, c, d, done | (1U << v), d_prefix + d[v]);
            best = std::min(best, next);
        }
    }
    return best;
}

void check(const std::vector<int> &parent, const std::vector<Weight> &c, const std::vector<Weight> &d,
           Weight expected)
{
    const int n = static_cast<int>(parent.size());
    const auto answer = minimum_inversion_topological_order(n, parent, c, d);
    assert(answer.inversions == expected && answer.order.size() == parent.size());
    std::vector<bool> seen(n);
    Weight value = 0, prefix = 0;
    for (int v : answer.order)
    {
        assert(0 <= v && v < n && !seen[v]);
        assert(parent[v] < 0 || seen[parent[v]]);
        value += prefix * c[v];
        prefix += d[v];
        seen[v] = true;
    }
    assert(value == answer.inversions);
}

template <class Exception>
void reject(int n, const std::vector<int> &parent,
            const std::vector<Weight> &c, const std::vector<Weight> &d)
{
    try
    {
        static_cast<void>(minimum_inversion_topological_order(n, parent, c, d));
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    check({}, {}, {}, 0);
    check({-1}, {0}, {0}, 0);
    check({-1, 0, 0}, {0, 2, 5}, {0, 1, 1}, 2);
    check({2, 2, -1, 0}, {1, 0, 0, 7}, {2, 0, 0, 1}, 14);
    check({-1, 0, 0, 2}, {0, 11, 20, 0}, {0, 3, 0, 0}, 0);
    check({-1, 0, 0}, {0, 1, 2}, {0, 1, 2}, 2);
    check({-1, 0, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, 0);

    std::mt19937 random(710018);
    for (int trial = 0; trial < 720; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 8);
        std::vector<int> parent(n, -1), vertex(n);
        std::iota(vertex.begin(), vertex.end(), 0);
        std::shuffle(vertex.begin(), vertex.end(), random);
        for (int v = 1; v < n; ++v)
        {
            parent[vertex[v]] = vertex[random() % v];
        }
        std::vector<Weight> c(n), d(n);
        for (int v = 0; v < n; ++v)
        {
            c[v] = random() % 13;
            d[v] = random() % 13;
        }
        check(parent, c, d, slow(parent, c, d));
    }

    constexpr auto kMax = std::numeric_limits<Weight>::max();
    check({-1, 0}, {0, 1}, {kMax, 0}, kMax);
    check({-1, 0, 0}, {0, kMax - 2, 2}, {0, kMax - 1, 1}, kMax - 2);
    check({-1, 0, 0}, {kMax, 0, 0}, {0, kMax - 1, 1}, 0);
    reject<std::overflow_error>(2, {-1, 0}, {kMax, 1}, {0, 0});
    reject<std::overflow_error>(2, {-1, 0}, {0, 0}, {kMax, 1});
    reject<std::overflow_error>(2, {-1, 0}, {0, 2}, {kMax, 0});
    reject<std::overflow_error>(3, {-1, 0, 1}, {0, 1, 1}, {kMax, 0, 0});

    reject<std::invalid_argument>(-1, {}, {}, {});
    reject<std::invalid_argument>(1, {}, {0}, {0});
    reject<std::invalid_argument>(1, {-1}, {}, {0});
    reject<std::invalid_argument>(1, {-1}, {0}, {});
#ifndef SKIP_RUNTIME_CHECKS
    reject<std::out_of_range>(1, {-2}, {0}, {0});
    reject<std::out_of_range>(2, {-1, 2}, {0, 0}, {0, 0});
    reject<std::invalid_argument>(1, {0}, {0}, {0});
    reject<std::invalid_argument>(2, {-1, -1}, {0, 0}, {0, 0});
    reject<std::invalid_argument>(3, {-1, 2, 1}, {0, 0, 0}, {0, 0, 0});
#endif

    constexpr int kSize = 10'000;
    std::vector<int> chain(kSize);
    std::iota(chain.begin(), chain.end(), -1);
    const std::vector<Weight> ones(kSize, 1);
    check(chain, ones, ones, static_cast<Weight>(kSize) * (kSize - 1) / 2);
}
