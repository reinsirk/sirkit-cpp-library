// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Component_Add_Union_Find.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
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
    using Data = sirkit::ComponentAddUnionFind<>;
    Data empty;
    assert(empty.size() == 0);
    Data zero(std::span<const std::int64_t>{});
    assert(zero.size() == 0);

    const std::vector<std::int64_t> initial{1, -2, 3, -4, 5, -6, 7, -8};
    Data values(initial);
    assert(values.size() == 8 && values.get(4) == 5 && values.component_sum(5) == -6);
    assert(values.root(5) == 5 && values.size(5) == 1);
    assert(values.unite(0, 1));
    values.add_component(1, 10);
    assert(values.unite(2, 3));
    values.add_component(2, -100);
    assert(values.unite(0, 2));
    assert(values.get(3) == -104 && values.get(1) == 8 && values.get(0) == 11);
    assert(values.component_sum(3) == -182);
    values.add_component(3, 2);
    assert(values.get(2) == -95 && values.get(3) == -102);
    assert(values.unite(4, 0));
    assert(values.size(4) == 5 && values.issame(4, 3) && !values.issame(4, 5));
    assert(!values.unite(1, 3));
    assert(values.get(4) == 5 && values.component_sum(4) == -169);
    auto copy = values;
    copy.add_component(4, 50);
    assert(copy.get(2) == -45 && values.get(2) == -95);
    assert(copy.component_sum(1) == 81 && values.component_sum(1) == -169);

    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    const std::vector<std::int64_t> wide{kMax, kMin, 0};
    Data wrapping(wide);
    assert(wrapping.unite(0, 1));
    wrapping.add_component(0, 1);
    assert(wrapping.get(0) == kMin && wrapping.get(1) == kMin + 1 && wrapping.component_sum(1) == 1);
    assert(wrapping.unite(2, 1));
    wrapping.add_component(2, kMin);
    assert(wrapping.get(0) == 0 && wrapping.get(1) == 1 && wrapping.get(2) == kMin);
    assert(wrapping.component_sum(0) == kMin + 1);

    const std::vector<std::int8_t> bytes{120, 110, -120};
    sirkit::ComponentAddUnionFind<std::int8_t> short_data(bytes);
    short_data.unite(0, 1);
    short_data.add_component(1, 20);
    short_data.unite(2, 1);
    assert(short_data.get(0) == -116 && short_data.get(1) == -126 && short_data.get(2) == -120);
    assert(short_data.component_sum(2) == -106);
    const std::vector<std::uint64_t> unsigned_values{std::numeric_limits<std::uint64_t>::max(), 1};
    sirkit::ComponentAddUnionFind<std::uint64_t> unsigned_data(unsigned_values);
    unsigned_data.unite(1, 0);
    unsigned_data.add_component(1, 3);
    assert(unsigned_data.get(0) == 2 && unsigned_data.get(1) == 4 && unsigned_data.component_sum(0) == 6);

    std::mt19937 random(1054119);
    for (int trial = 0; trial < 740; ++trial)
    {
        const int n = static_cast<int>(random() % 30) + 1;
        std::vector<int> group(n);
        std::iota(group.begin(), group.end(), 0);
        std::vector<std::int64_t> model(n);
        for (auto &x : model)
        {
            x = static_cast<int>(random() % 61) - 30;
        }
        Data actual(model);
        for (int step = 0; step < 200; ++step)
        {
            const int u = static_cast<int>(random() % n);
            const int v = static_cast<int>(random() % n);
            if (random() % 4 == 0)
            {
                const int from = group[v];
                const int into = group[u];
                assert(actual.unite(u, v) == (from != into));
                for (int &g : group)
                {
                    if (g == from)
                    {
                        g = into;
                    }
                }
            }
            else
            {
                const int delta = static_cast<int>(random() % 81) - 40;
                actual.add_component(u, delta);
                for (int w = 0; w < n; ++w)
                {
                    if (group[w] == group[u])
                    {
                        model[w] += delta;
                    }
                }
            }

            assert(actual.issame(u, v) == (group[u] == group[v]));
            std::int64_t sum = 0;
            int count = 0;
            const int leader = actual.root(u);
            for (int w = 0; w < n; ++w)
            {
                assert(actual.get(w) == model[w]);
                if (group[w] == group[u])
                {
                    assert(actual.root(w) == leader);
                    sum += model[w];
                    ++count;
                }
            }
            assert(actual.component_sum(u) == sum && actual.size(u) == count);
        }
    }

    const auto maximum = static_cast<std::size_t>(std::numeric_limits<int>::max());
    assert(sirkit::detail::checked_component_add_uf_size(maximum) == maximum);
    check_error<std::length_error>([&] { (void)sirkit::detail::checked_component_add_uf_size(maximum + 1); });
    check_error<std::length_error>([] { Data bad(-1); });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int bad : {-1, 8})
    {
        check_error<std::out_of_range>([&] { values.unite(0, bad); });
        check_error<std::out_of_range>([&] { values.add_component(bad, 3); });
        check_error<std::out_of_range>([&] { (void)values.get(bad); });
    }
#endif
}
