// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Topological_Order_Counter.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &edges)
{
    const sirkit::TopologicalOrderCounter orders(n, edges);
    assert(orders.size() == n);
    std::vector<int> perm(n), position(n);
    std::iota(perm.begin(), perm.end(), 0);
    std::uint64_t rank = 0;
    do
    {
        for (int i = 0; i < n; ++i)
        {
            position[perm[i]] = i;
        }
        const bool valid = std::ranges::all_of(edges, [&](Edge edge)
                                              { return position[edge.first] < position[edge.second]; });
        if (valid)
        {
            assert(orders.order_at(rank) == perm);
            assert(orders.rank(perm) == rank);
            ++rank;
        }
        else
        {
            assert(!orders.rank(perm));
        }
    } while (std::ranges::next_permutation(perm).found);
    assert(orders.count() == rank);
    assert(!orders.order_at(rank));
}

template <typename Error, typename Func>
void check_error(Func func)
{
    bool found = false;
    try
    {
        func();
    }
    catch (const Error &)
    {
        found = true;
    }
    assert(found);
}
} // namespace

int main()
{
    check(0, {});
    check(1, {});
    check(2, {{0, 1}, {0, 1}});
    check(2, {{0, 0}});
    check(3, {{0, 1}, {1, 2}, {2, 0}});
    check(5, {});
    std::mt19937 random(443157);
    for (int test = 0; test < 350; ++test)
    {
        const int n = static_cast<int>(random() % 8) + 1;
        std::vector<int> perm(n);
        std::iota(perm.begin(), perm.end(), 0);
        std::shuffle(perm.begin(), perm.end(), random);
        std::vector<Edge> edges;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                if (random() % 3 == 0)
                {
                    edges.emplace_back(perm[u], perm[v]);
                }
            }
        }
        if (test % 4 == 0 && !edges.empty())
        {
            const auto [u, v] = edges.front();
            edges.emplace_back(v, u);
        }
        check(n, edges);
    }
    const sirkit::TopologicalOrderCounter largest(20, {});
    constexpr std::uint64_t kFactorial20 = 2432902008176640000;
    assert(largest.count() == kFactorial20);
    std::vector<int> reverse(20);
    std::iota(reverse.rbegin(), reverse.rend(), 0);
    assert(largest.order_at(kFactorial20 - 1) == reverse);
    assert(largest.rank(reverse) == kFactorial20 - 1);
    const auto witness = largest.order_at(1972987208176640011);
    assert(witness && largest.rank(*witness) == 1972987208176640011);
    assert(!largest.order_at(kFactorial20));
    assert(!largest.rank({}));

    const sirkit::TopologicalOrderCounter two(2, {});
    assert(!two.rank(std::array{0, 0}));
    assert(!two.rank(std::array{-1, 0}));
    assert(!two.rank(std::array{0, 2}));
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::length_error>([] { (void)sirkit::TopologicalOrderCounter(-1, {}); });
    check_error<std::length_error>([] { (void)sirkit::TopologicalOrderCounter(21, {}); });
    for (const Edge edge : std::vector<Edge>{{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        check_error<std::out_of_range>([&] { (void)sirkit::TopologicalOrderCounter(2, std::span(&edge, 1)); });
    }
#endif
}
