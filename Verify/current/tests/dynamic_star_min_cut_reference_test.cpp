// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dynamic_Star_Min_Cut.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Cut = sirkit::DynamicStarMinCut;
using Edge = sirkit::DynamicStarCutEdge;

template<class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_limits()
{
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    Cut singleton(1, {});
    assert(singleton.size() == 1 && singleton.minimum_cut() == 0 && singleton.star_weight(0) == 0);
    assert(singleton.set_star_weight(0, kMax) == kMax);
    assert(singleton.set_star_weight(0, kMax) == kMax);
    assert(singleton.set_star_weight(0, 0) == 0);

    const std::array edges{Edge{0, 1, kMax - 2}, Edge{0, 0, kMax}};
    const std::array<std::uint64_t, 2> star{1, 1};
    Cut cut(2, edges, star);
    assert(cut.minimum_cut() == 2);
    must_throw<std::overflow_error>([&] { static_cast<void>(cut.set_star_weight(1, 2)); });
    assert(cut.minimum_cut() == 2 && cut.star_weight(1) == 1);
    auto copied = cut;
    assert(copied.set_star_weight(0, 0) == 1);
    assert(cut.minimum_cut() == 2);
    singleton = copied;
    assert(singleton.minimum_cut() == 1 && singleton.star_weight(0) == 0);
    singleton = std::move(cut);
    assert(singleton.minimum_cut() == 2 && cut.minimum_cut() == 0);
    assert(singleton.set_star_weight(1, 0) == 1);
    assert(singleton.set_star_weight(1, 1) == 2);
    assert(singleton.set_star_weight(0, 0) == 1);

    must_throw<std::length_error>([] { const Cut invalid(0, {}); });
    must_throw<std::length_error>([] { const Cut invalid(std::numeric_limits<int>::max() / 2 + 1, {}); });
    must_throw<std::length_error>([] { static_cast<void>(sirkit::detail::star_cut_arc_slots(8, 15)); });
    must_throw<std::overflow_error>([] {
        const std::array both{Edge{0, 1, kMax}, Edge{1, 0, 1}};
        const Cut invalid(2, both);
    });
    must_throw<std::overflow_error>([] {
        const std::array<std::uint64_t, 2> invalid_star{kMax, 1};
        const Cut invalid(2, {}, invalid_star);
    });
    must_throw<std::overflow_error>([] {
        const std::array heavy{Edge{0, 1, kMax}};
        const std::array<std::uint64_t, 2> invalid_star{0, 1};
        const Cut invalid(2, heavy, invalid_star);
    });
#ifndef SKIP_RUNTIME_CHECKS
    must_throw<std::invalid_argument>([] {
        const std::array<std::uint64_t, 1> wrong_size{1};
        const Cut invalid(2, {}, wrong_size);
    });
    for (const auto &[u, v] : {std::pair{-1, 0}, std::pair{2, 0}, std::pair{0, -1}, std::pair{0, 2}})
    {
        must_throw<std::out_of_range>([&] {
            const std::array invalid_edges{Edge{u, v, 1}};
            const Cut invalid(2, invalid_edges);
        });
    }
    for (const int v : {-1, 2})
    {
        must_throw<std::out_of_range>([&] { static_cast<void>(singleton.star_weight(v)); });
    }
#endif
}

std::vector<std::uint64_t> cut_weights(int n, std::span<const Edge> edges)
{
    std::vector<std::uint64_t> weights(1U << n);
    for (unsigned mask = 1; mask < weights.size(); ++mask)
    {
        for (const auto &[u, v, weight] : edges)
        {
            if (((mask >> u) ^ (mask >> v)) & 1)
            {
                weights[mask] += weight;
            }
        }
    }
    return weights;
}

std::uint64_t brute(const std::vector<std::uint64_t> &base, const std::vector<std::uint64_t> &star)
{
    auto best = std::numeric_limits<std::uint64_t>::max();
    std::vector<std::uint64_t> sum(base.size());
    for (unsigned mask = 1; mask < base.size(); ++mask)
    {
        sum[mask] = sum[mask & (mask - 1)] + star[std::countr_zero(mask)];
        best = std::min(best, base[mask] + sum[mask]);
    }
    return best;
}

void check_random()
{
    std::mt19937 rng(570);
    for (int test = 0; test < 800; ++test)
    {
        const int n = 1 + static_cast<int>(rng() % 11);
        std::vector<Edge> edges;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u; v < n; ++v)
            {
                if (rng() % 100 < static_cast<unsigned>(test % 105))
                {
                    edges.push_back({u, v, rng() % 19});
                    if (rng() % 7 == 0)
                    {
                        edges.push_back({v, u, rng() % 11});
                    }
                }
            }
        }
        std::vector<std::uint64_t> star(n);
        for (auto &weight : star)
        {
            weight = rng() % 23;
        }
        Cut cut(n, edges, star);
        const auto base = cut_weights(n, edges);
        assert(cut.minimum_cut() == brute(base, star));
        for (int q = 0; q < 30; ++q)
        {
            const int u = static_cast<int>(rng() % n);
            star[u] = q % 3 == 0 ? 0 : rng() % 53;
            const auto minimum = cut.set_star_weight(u, star[u]);
            assert(cut.star_weight(u) == star[u] && cut.minimum_cut() == minimum);
            assert(minimum == brute(base, star));
        }
    }
}
}

int main()
{
    check_limits();
    check_random();
}
