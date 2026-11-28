// SPDX-License-Identifier: CC0-1.0
#include "DataStructure/Persistent_Weighted_Union_Find.cpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <optional>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Weighted = sirkit::PersistentWeightedUnionFind<std::int64_t>;
using Status = Weighted::MergeResult;

void Expect(bool value)
{
    if (!value)
    {
        std::abort();
    }
}

template <class Error, class Fn>
void ExpectThrow(Fn function)
{
    try
    {
        function();
    }
    catch (const Error &)
    {
        return;
    }
    std::abort();
}

using Graph = std::vector<std::vector<std::pair<int, int>>>;

std::vector<std::optional<std::int64_t>> Oracle(const Graph &g, int source)
{
    std::queue<int> queue;
    std::vector<std::optional<std::int64_t>> weights(g.size());
    weights[source] = 0;
    queue.push(source);
    while (!queue.empty())
    {
        const int u = queue.front();
        queue.pop();
        for (const auto &[v, cost] : g[u])
        {
            if (!weights[v])
            {
                weights[v] = *weights[u] + cost;
                queue.push(v);
            }
        }
    }
    return weights;
}

void CheckBranching()
{
    std::mt19937 random(772639);
    for (int iteration = 0; iteration < 100; ++iteration)
    {
        const int n = 2 + random() % 79;
        Weighted uf(n);
        Expect(uf.size() == n && uf.version() == nullptr);
        std::vector<Weighted::Version> versions{uf.version()};
        std::vector<Graph> history{Graph(n)};
        for (int i = 0; i < 150; ++i)
        {
            const std::size_t b = (i % 4 == 0) ? random() % history.size() : history.size() - 1;
            const int u = random() % n;
            const int v = random() % n;
            const auto before = Oracle(history[b], u);
            const int cost = before[v] && i % 5 == 0 ? static_cast<int>(*before[v]) : static_cast<int>(random() % 2001) - 1000;
            Expect(uf.try_diff(u, v, versions[b]) == before[v]);
            Expect(uf.issame(u, v, versions[b]) == before[v].has_value());
            Graph result = history[b];
            const Status expected = !before[v] ? Status::Merged : *before[v] == cost ? Status::AlreadyConsistent
                                                                                     : Status::Contradiction;
            Expect(uf.unite_checked(u, v, cost, versions[b]) == expected);
            if (!before[v])
            {
                result[u].emplace_back(v, cost);
                result[v].emplace_back(u, -cost);
                Expect(uf.version() != versions[b]);
            }
            else
            {
                Expect(uf.version() == versions[b]);
            }
            const auto after = Oracle(result, v);
            Expect(uf.try_diff(v, u) == after[u]);
            const auto [representative, delta] = uf.root_with_weight(v);
            Expect(uf.root(v) == representative && uf.root_with_weight(v, uf.version()).second == delta);
            Expect(after[representative] && -*after[representative] == delta);
            int count = 0;
            for (int j = 0; j < n; ++j)
            {
                count += after[j].has_value();
            }
            Expect(uf.size(v) == count && uf.size(v, uf.version()) == count);
            Expect(uf.same(u, v) && uf.issame(u, v));
            versions.push_back(uf.version());
            history.push_back(std::move(result));
        }
    }
}

void CheckOverflowAndLifetime()
{
    constexpr auto max = std::numeric_limits<std::int64_t>::max();
    constexpr auto min = std::numeric_limits<std::int64_t>::min();
    Weighted uf(4);
    const auto initial = uf.version();
    Expect(uf.unite_checked(0, 1, max) == Status::Merged);
    const auto maximal = uf.version();
    ExpectThrow<std::overflow_error>([&]
                                     {
                                         uf.unite_checked(1, 2, 1);
                                     });
    Expect(uf.version() == maximal && !uf.same(1, 2));
    Expect(uf.unite_checked(0, 2, min) == Status::Merged);
    const auto limits = uf.version();
    ExpectThrow<std::overflow_error>([&]
                                     {
                                         uf.unite_checked(2, 3, -1);
                                     });
    ExpectThrow<std::overflow_error>([&]
                                     {
                                         (void)uf.try_diff(1, 2);
                                     });
    ExpectThrow<std::overflow_error>([&]
                                     {
                                         (void)uf.try_diff(2, 1);
                                     });
    Expect(uf.version() == limits);
    Expect(uf.root(0) == uf.root(2) && uf.size(2) == 3);
    Expect(uf.try_diff(0, 2) == min && uf.try_diff(0, 1) == max);

    Weighted copy = uf;
    copy.restore(initial);
    Expect(!copy.issame(0, 1) && uf.version() == limits);
    Expect(copy.unite_checked(0, 3, 7) == Status::Merged);
    Expect(uf.try_diff(0, 3, copy.version()) == 7);
    Weighted moved(std::move(copy));
    Expect(copy.size() == 0 && copy.version() == nullptr && moved.try_diff(0, 3) == 7);
    copy = moved;
    moved = Weighted(0);
    Expect(moved.size() == 0 && copy.try_diff(0, 3) == 7);

    ExpectThrow<std::length_error>([]
                                   {
                                       Weighted huge(std::numeric_limits<std::size_t>::max());
                                   });
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)uf.root(-1);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)uf.try_diff(0, 4);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)moved.size(0);
                                   });
    Weighted other(4);
    ExpectThrow<std::invalid_argument>([&]
                                       {
                                           other.restore(limits);
                                       });
    ExpectThrow<std::invalid_argument>([&]
                                       {
                                           (void)other.try_diff(0, 1, limits);
                                       });
    Expect(other.version() == nullptr);
#endif

    using Modular = sirkit::PersistentWeightedUnionFind<std::uint8_t>;
    Modular mod(3);
    Expect(mod.unite_checked(0, 1, 255) == Modular::MergeResult::Merged);
    Expect(mod.unite_checked(1, 2, 2) == Modular::MergeResult::Merged);
    Expect(mod.try_diff(0, 2) == 1 && mod.try_diff(2, 0) == 255);
}

struct Value
{
    explicit Value(int v)
        : value(v)
    {
    }

    bool operator==(const Value &) const = default;
    int value;
};
struct Add
{
    int modulus;

    Value operator()(const Value &a, const Value &b) const
    {
        return Value((a.value + b.value) % modulus);
    }
};
struct Subtract
{
    int modulus;

    Value operator()(const Value &a, const Value &b) const
    {
        return Value((a.value + (modulus - b.value)) % modulus);
    }
};

void CheckCustomGroup()
{
    using Modular = sirkit::PersistentWeightedUnionFind<Value, Add, Subtract, 2>;
    Modular uf(20, Value(0), Add{13}, Subtract{13});
    Expect(uf.unite_checked(5, 6, Value(10)) == Modular::MergeResult::Merged);
    const auto base = uf.version();
    Expect(uf.unite_checked(17, 6, Value(3)) == Modular::MergeResult::Merged);
    Expect(uf.try_diff(17, 5) == Value(6) && uf.root(17) == 5);
    Expect(uf.unite_checked(5, 6, Value(3), base) == Modular::MergeResult::Contradiction);
    Expect(!uf.try_diff(6, 17) && uf.version() == base);
}
}

int main()
{
    CheckBranching();
    CheckOverflowAndLifetime();
    CheckCustomGroup();
}
