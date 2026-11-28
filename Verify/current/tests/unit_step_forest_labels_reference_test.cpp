// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Unit_Step_Forest_Labels.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;
using Interval = sirkit::IntegerLabelInterval;

bool close(std::int64_t a, std::int64_t b)
{
    if (a < b)
    {
        std::swap(a, b);
    }
    return static_cast<std::uint64_t>(a) - static_cast<std::uint64_t>(b) == 1;
}

void check_result(const std::vector<Edge> &edges, const std::vector<Interval> &allowed, const std::vector<std::int64_t> &labels)
{
    assert(labels.size() == allowed.size());
    for (std::size_t v = 0; v < allowed.size(); ++v)
    {
        assert(allowed[v].lower <= labels[v] && labels[v] <= allowed[v].upper);
    }
    for (const auto &[u, v] : edges)
    {
        assert(close(labels[u], labels[v]));
    }
}

void check(const std::vector<Edge> &edges, const std::vector<Interval> &allowed)
{
    const auto n = static_cast<int>(allowed.size());
    std::vector<std::vector<int>> constraints(n);
    for (const auto &[u, v] : edges)
    {
        constraints[std::max(u, v)].push_back(std::min(u, v));
    }
    std::vector<std::int64_t> assigned(n);
    auto enumerate = [&](auto &self, int v) -> bool
    {
        if (v == n)
        {
            return true;
        }
        for (auto value = allowed[v].lower; value <= allowed[v].upper; ++value)
        {
            const bool valid = std::all_of(constraints[v].begin(), constraints[v].end(),
                                           [&](int u) { return close(assigned[u], value); });
            if (valid)
            {
                assigned[v] = value;
                if (self(self, v + 1))
                {
                    return true;
                }
            }
        }
        return false;
    };
    const bool possible = enumerate(enumerate, 0);
    const auto labels = sirkit::unit_step_forest_labels(n, edges, allowed);
    assert(labels.has_value() == possible);
    if (labels)
    {
        check_result(edges, allowed, *labels);
    }
}

template <typename Error, typename Action>
void check_error(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    check({}, {});
    check({}, {{2, 1}});
    check({}, {{-5, -5}, {6, 6}});
    check({{0, 1}, {1, 2}}, {{0, 0}, {-10, 10}, {2, 2}});
    check({{1, 0}, {1, 2}}, {{0, 0}, {-10, 10}, {4, 6}});
    check({{0, 1}}, {{0, 0}, {-1, -1}});
    check({{0, 1}}, {{0, 0}, {1, 1}});
    check({{0, 1}}, {{1, 1}, {-1, -1}});

    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    const std::vector<Edge> wide_edges{{0, 1}, {1, 2}, {3, 4}, {4, 5}};
    const std::vector<Interval> wide{{kMax, kMax}, {}, {kMax, kMax}, {kMin, kMin}, {}, {kMin, kMin}};
    const auto wide_result = sirkit::unit_step_forest_labels(6, wide_edges, wide);
    assert(wide_result);
    check_result(wide_edges, wide, *wide_result);
    assert((*wide_result)[1] == kMax - 1 && (*wide_result)[4] == kMin + 1);
    auto impossible = wide;
    impossible[1] = {kMin, kMin};
    assert(!sirkit::unit_step_forest_labels(6, wide_edges, impossible));

    std::mt19937 random(1462283);
    for (int test = 0; test < 1050; ++test)
    {
        const int n = static_cast<int>(random() % 8) + 1;
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (random() % 4 != 0)
            {
                const int parent = static_cast<int>(random() % v);
                edges.emplace_back(random() % 2 ? Edge{v, parent} : Edge{parent, v});
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        std::vector<Interval> allowed(n);
        for (auto &interval : allowed)
        {
            const int low = static_cast<int>(random() % 9) - 4;
            interval = {low, low + static_cast<int>(random() % 6) - 1};
        }
        check(edges, allowed);
    }

    check_error<std::length_error>([] { (void)sirkit::unit_step_forest_labels(-1, {}, {}); });
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::invalid_argument>([] { (void)sirkit::unit_step_forest_labels(1, {}, {}); });
    const std::vector<Interval> intervals(3);
    for (const Edge &edge : std::vector<Edge>{{-1, 0}, {3, 0}, {0, -1}, {0, 3}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::unit_step_forest_labels(3, std::span(&edge, 1), intervals); });
    }
    for (const auto &edges : std::vector<std::vector<Edge>>{{{0, 0}}, {{0, 1}, {1, 0}}, {{0, 1}, {0, 2}, {2, 1}}})
    {
        check_error<std::invalid_argument>(
            [&] { (void)sirkit::unit_step_forest_labels(3, edges, intervals); });
    }
#endif
}
