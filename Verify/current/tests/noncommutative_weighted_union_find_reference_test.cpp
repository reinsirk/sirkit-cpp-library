// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Noncommutative_Weighted_Union_Find.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Perm = std::array<std::size_t, 4>;

constexpr Perm Compose(const Perm &a, const Perm &b)
{
    return {a[b[0]], a[b[1]], a[b[2]], a[b[3]]};
}

constexpr Perm Identity()
{
    return {0, 1, 2, 3};
}

constexpr Perm Inverse(const Perm &a)
{
    Perm result{};
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[a[i]] = i;
    }
    return result;
}

using UF = sirkit::NoncommutativeWeightedUnionFind<Perm, Compose, Identity, Inverse>;
using Status = UF::MergeResult;

bool throw_inverse = false;
Perm ThrowingInverse(const Perm &value)
{
    if (throw_inverse)
    {
        throw std::runtime_error("inverse");
    }
    return Inverse(value);
}

struct Arc
{
    int to;
    Perm relation;
};

std::optional<Perm> Path(const std::vector<std::vector<Arc>> &graph, int from, int to)
{
    std::queue<int> queue;
    std::vector<std::optional<Perm>> seen(graph.size());
    queue.push(from);
    seen[from] = Identity();
    while (!queue.empty())
    {
        const auto v = queue.front();
        queue.pop();
        if (v == to)
        {
            return seen[v];
        }
        for (const auto &[u, relation] : graph[v])
        {
            if (!seen[u])
            {
                seen[u] = Compose(*seen[v], relation);
                queue.push(u);
            }
        }
    }
    return std::nullopt;
}
}

int main()
{
    UF empty;
    assert(empty.empty() && empty.size() == 0 && empty.component_count() == 0);
    UF uf(6);
    constexpr Perm p{1, 0, 2, 3}, q{0, 2, 1, 3};
    static_assert(Compose(p, q) != Compose(q, p));
    assert(!uf.issame(0, 1) && uf.consistent(0, 1, p));
    assert(!uf.try_diff(0, 1));
    assert(uf.merge(0, 1, p));
    assert(uf.unite_checked(2, 1, Inverse(q)) == Status::Merged);
    assert(uf.leader(1) == uf.root(2) && uf.size(0) == 3);
    assert(uf.diff(0, 2) == Compose(p, q));
    assert(uf.diff(2, 0) == Compose(Inverse(q), Inverse(p)));
    const auto [root, offset] = uf.root_with_weight(2);
    assert(root == uf.root(0) && offset == uf.weight(2) && uf.weight(root) == Identity());
    assert(uf.constrain(2, 0, Compose(Inverse(q), Inverse(p))));
    assert(!uf.constrain(0, 2, Compose(q, p)));
    assert(!uf.consistent(0, 2, Compose(q, p)));
    assert(!uf.merge(0, 1, p));
    uf.unite(0, 2, Compose(q, p));
    assert(uf.diff(0, 2) == Compose(p, q));

    auto copy = uf;
    copy.unite(3, 4, p);
    assert(!uf.same(3, 4));
    auto moved = std::move(copy);
    assert(copy.empty() && moved.same(3, 4));
    auto &move_alias = moved;
    moved = std::move(move_alias);
    copy = std::move(moved);
    assert(moved.empty() && copy.same(3, 4));
    moved = copy;
    const auto &copy_alias = moved;
    moved = copy_alias;
    assert(moved.component_count() == copy.component_count());

    using ThrowingUF = sirkit::NoncommutativeWeightedUnionFind<Perm, Compose, Identity, ThrowingInverse>;
    ThrowingUF interrupted(3);
    assert(interrupted.merge(0, 1, p));
    throw_inverse = true;
    try
    {
        interrupted.constrain(2, 1, q);
        assert(false);
    }
    catch (const std::runtime_error &)
    {
    }
    throw_inverse = false;
    assert(!interrupted.same(0, 2) && interrupted.diff(0, 1) == p);
    assert(interrupted.constrain(2, 1, q));

    std::mt19937 rng(629875);
    for (int trial = 0; trial < 480; ++trial)
    {
        const auto n = trial % 23 + 1;
        UF candidate(n);
        std::vector<std::vector<Arc>> graph(n);
        int components = n;
        for (int step = 0; step < 140; ++step)
        {
            const auto from = static_cast<int>(rng() % n);
            const auto to = static_cast<int>(rng() % n);
            const auto expected = Path(graph, from, to);
            assert(candidate.try_diff(from, to) == expected);
            assert(candidate.issame(from, to) == expected.has_value());
            if (expected)
            {
                assert(candidate.diff(to, from) == *Path(graph, to, from));
            }
            if (step % 3 != 2)
            {
                Perm value = Identity();
                std::shuffle(value.begin(), value.end(), rng);
                if (expected && step % 4 == 0)
                {
                    value = *expected;
                }
                assert(candidate.consistent(from, to, value) == (!expected || value == *expected));
                const auto result = candidate.unite_checked(from, to, value);
                if (!expected)
                {
                    assert(result == Status::Merged);
                    graph[from].push_back({to, value});
                    graph[to].push_back({from, Inverse(value)});
                    --components;
                }
                else
                {
                    assert(result == (*expected == value ? Status::AlreadyConsistent : Status::Contradiction));
                    assert(candidate.diff(from, to) == *expected);
                }
            }
            assert(candidate.component_count() == components);
            const auto [representative, relative] = candidate.root_with_weight(to);
            assert(Path(graph, representative, to) == relative);
            assert(candidate.diff(to, to) == Identity());
        }
        for (int from = 0; from < n; ++from)
        {
            int component_size = 0;
            for (int to = 0; to < n; ++to)
            {
                const auto expected = Path(graph, from, to);
                assert(candidate.try_diff(from, to) == expected);
                component_size += expected.has_value();
            }
            assert(candidate.size(from) == component_size);
        }
    }

    try
    {
        const UF too_large(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (int vertex : {-1, 6})
    {
        try
        {
            (void)uf.root(vertex);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    try
    {
        (void)uf.diff(0, 3);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
