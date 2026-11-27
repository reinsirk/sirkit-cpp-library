// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Persistent_Affine_Range_Sum.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;
using Tree = sirkit::PersistentAffineRangeSum<kMod>;

std::uint32_t Normalize(std::int64_t value)
{
    const auto residue = value % static_cast<std::int64_t>(kMod);
    return static_cast<std::uint32_t>((residue + kMod) % kMod);
}
}

int main()
{
    Tree empty;
    const auto empty_root = empty.initial_root();
    assert(empty.size() == 0 && empty.allocated_nodes() == 0);
    assert(empty.all_sum(empty_root) == 0 && empty.range_sum(empty_root, 0, 0) == 0);
    assert(empty.all_sum(empty.range_affine(empty_root, 0, 0, 0, 17)) == 0);

    const std::array<std::int64_t, 8> values{1, 2, 3, 4, 5, 6, 7, 8};
    Tree tree(values);
    const auto initial = tree.initial_root();
    assert(tree.size() == 8 && tree.allocated_nodes() == 15);
    const auto a = tree.range_affine(initial, 0, 8, 3, 1);
    const auto b = tree.range_affine(a, 0, 8, 2, 5);
    assert(tree.allocated_nodes() == 15);
    assert(tree.range_sum(a, 1, 4) == 30);
    assert(tree.range_sum(b, 0, 1) == 13);
    assert(tree.range_sum(b, 7, 8) == 55);

    const auto partial = tree.range_affine(a, 2, 7, 0, -7);
    const auto mixed = tree.range_copy(partial, b, 1, 4);
    const auto &read = tree;
    assert(tree.range_sum(mixed, 0, 1) == 4);
    assert(tree.range_sum(mixed, 1, 4) == 75);
    assert(tree.range_sum(mixed, 4, 7) == Normalize(-21));
    assert(tree.range_sum(mixed, 7, 8) == 25);
    assert(tree.all_sum(partial) == 1);
    assert(read.all_sum(a) == 116 && read.all_sum(b) == 272 && read.all_sum(initial) == 36);
    const auto count = tree.allocated_nodes();
    assert(tree.all_sum(tree.range_copy(initial, b, 0, 8)) == 272);
    assert(tree.all_sum(tree.range_copy(a, b, 2, 2)) == 116);
    assert(tree.all_sum(tree.range_copy(a, a, 1, 5)) == 116);
    assert(tree.all_sum(tree.range_affine(a, 2, 6, 1, 0)) == 116);
    assert(tree.range_sum(a, 5, 5) == 0);
    assert(tree.allocated_nodes() == count);

    const auto different_offset = tree.range_affine(initial, 0, 8, 3, 9);
    assert(tree.all_sum(tree.range_copy(a, different_offset, 2, 6)) == 148);
    assert(tree.all_sum(a) == 116);

    auto shared = tree;
    auto old = tree.range_affine(initial, 0, 1, 1, 13);
    assert(shared.all_sum(old) == 49);
    const auto copied = shared.range_copy(initial, old, 0, 4);
    assert(tree.all_sum(copied) == 49 && tree.allocated_nodes() == shared.allocated_nodes());
    auto moved = std::move(shared);
    assert(shared.size() == 0 && shared.allocated_nodes() == 0 && shared.all_sum(shared.initial_root()) == 0);
    assert(moved.all_sum(old) == 49);

    constexpr auto kLarge = std::numeric_limits<std::uint32_t>::max();
    using LargeMod = sirkit::PersistentAffineRangeSum<kLarge>;
    const std::array<std::int64_t, 5> large_values{0, 0, 0, 0, kLarge - 1};
    LargeMod large(large_values);
    const auto large_root = large.range_affine(large.initial_root(), 0, 5, kLarge - 1, kLarge - 1);
    assert(large.all_sum(large_root) == kLarge - 4);
    assert(large.range_sum(large_root, 0, 4) == kLarge - 4);
    sirkit::PersistentAffineRangeSum<1> unit(values);
    assert(unit.all_sum(unit.range_affine(unit.initial_root(), 0, 8, 12, 19)) == 0);

    std::mt19937_64 rng(6779777);
    for (int trial = 0; trial < 440; ++trial)
    {
        const int n = trial % 59 + 1;
        std::vector<std::int64_t> start(n);
        for (auto &x : start)
        {
            x = static_cast<std::int64_t>(rng() % 10'001) - 5000;
        }
        Tree candidate(start);
        std::vector<Tree::Root> roots{candidate.initial_root()};
        std::vector<std::vector<std::uint32_t>> versions(1);
        for (const auto x : start)
        {
            versions[0].push_back(Normalize(x));
        }

        for (int step = 0; step < 235; ++step)
        {
            const auto dst = rng() % roots.size();
            const auto src = rng() % roots.size();
            const auto [left, right] = std::minmax(
                {static_cast<int>(rng() % (n + 1)), static_cast<int>(rng() % (n + 1))});
            auto model = versions[dst];
            if (step % 3 == 0)
            {
                const auto a = static_cast<std::int64_t>(rng() % 23) - 11;
                const auto b = static_cast<std::int64_t>(rng() % 301) - 150;
                roots.push_back(candidate.range_affine(roots[dst], left, right, a, b));
                for (int i = left; i < right; ++i)
                {
                    model[i] = static_cast<std::uint32_t>((std::uint64_t{Normalize(a)} * model[i] + Normalize(b)) % kMod);
                }
            }
            else if (step % 3 == 1)
            {
                roots.push_back(candidate.range_copy(roots[dst], roots[src], left, right));
                std::copy(versions[src].begin() + left, versions[src].begin() + right, model.begin() + left);
            }
            else
            {
                roots.push_back(roots[dst]);
            }
            versions.push_back(model);
            const auto nodes = candidate.allocated_nodes();
            const auto old_sum = std::accumulate(versions[dst].begin() + left, versions[dst].begin() + right,
                                                 std::uint64_t{0}) %
                                 kMod;
            const auto new_sum = std::accumulate(model.begin(), model.end(), std::uint64_t{0}) % kMod;
            const auto new_partial = std::accumulate(model.begin() + left, model.begin() + right,
                                                     std::uint64_t{0}) %
                                     kMod;
            const auto &const_candidate = candidate;
            assert(const_candidate.range_sum(roots[dst], left, right) == old_sum);
            assert(const_candidate.all_sum(roots.back()) == new_sum);
            assert(const_candidate.range_sum(roots.back(), left, right) == new_partial);
            assert(candidate.allocated_nodes() == nodes);
        }
    }

    assert(sirkit::detail::CheckedPersistentAffineSize(10) == 10);
    assert(sirkit::detail::CheckedPersistentAffineBudget(3, 5, 8) == 8);
    try
    {
        (void)sirkit::detail::CheckedPersistentAffineSize(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    try
    {
        (void)sirkit::detail::CheckedPersistentAffineBudget(3, 6, 8);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    Tree foreign(values);
    try
    {
        (void)tree.range_copy(initial, foreign.initial_root(), 0, 1);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    for (const auto &[left, right] : {std::pair{-1, 4}, std::pair{5, 2}, std::pair{0, 9}})
    {
        try
        {
            (void)tree.range_sum(initial, left, right);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
