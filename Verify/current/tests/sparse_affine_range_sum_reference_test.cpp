// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Sparse_Affine_Range_Sum.cpp"

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
constexpr auto kMod = std::uint32_t{998244353};

std::uint32_t Normalize(std::int64_t value)
{
    const auto residue = value % static_cast<std::int64_t>(kMod);
    return static_cast<std::uint32_t>((residue + kMod) % kMod);
}
}

int main()
{
    using Tree = sirkit::SparseAffineRangeSum<kMod>;
    Tree empty;
    assert(empty.size() == 0 && empty.all_sum() == 0 && empty.range_sum(0, 0) == 0);
    empty.range_affine(0, 0, 3, -2);
    assert(empty.stored_nodes() == 0);

    constexpr auto kLength = std::numeric_limits<std::uint64_t>::max();
    Tree data(kLength, -4);
    assert(data.size() == kLength && data.stored_nodes() == 0);
    assert(data.all_sum() == std::uint64_t{Normalize(-4)} * (kLength % kMod) % kMod);
    data.range_affine(0, kLength, 0, 7);
    data.range_affine(1, kLength - 1, 1, 0);
    data.range_affine(1, 2, 0, 7);
    assert(data.stored_nodes() == 0);
    data.range_affine(kLength - 3, kLength - 1, 2, 3);
    const auto first_count = data.stored_nodes();
    assert(first_count != 0 && data.range_sum(kLength - 4, kLength) == 48);
    assert(data.all_sum() == (7 * (kLength % kMod) + 20) % kMod);
    data.range_affine(0, kLength, 3, 5);
    assert(data.range_sum(kLength - 4, kLength) == 164);
    assert(data.stored_nodes() == first_count);
    data.range_affine(kLength - 2, kLength, -1, 10);
    assert(data.range_sum(kLength - 3, kLength - 2) == 56);
    assert(data.range_sum(kLength - 2, kLength - 1) == Normalize(-46));
    assert(data.range_sum(kLength - 1, kLength) == Normalize(-16));

    auto copy = data;
    copy.range_affine(0, kLength, 0, 2);
    assert(copy.range_sum(kLength - 3, kLength) == 6);
    assert(data.range_sum(kLength - 3, kLength - 2) == 56);
    auto moved = std::move(copy);
    auto &move_alias = moved;
    moved = std::move(move_alias);
    assert(moved.range_sum(kLength - 2, kLength) == 4 && copy.size() == 0);
    copy = moved;
    const auto &copy_alias = copy;
    copy = copy_alias;
    assert(copy.all_sum() == moved.all_sum());
    moved = std::move(copy);
    assert(copy.size() == 0 && moved.size() == kLength);

    constexpr auto kLarge = std::numeric_limits<std::uint32_t>::max();
    using LargeMod = sirkit::SparseAffineRangeSum<kLarge>;
    LargeMod large(kLarge - 1, 1);
    large.range_affine(0, 1, 0, 0);
    large.range_affine(1, 2, 0, 2);
    assert(large.all_sum() == kLarge - 1);
    large.range_affine(0, large.size(), kLarge - 1, kLarge - 1);
    assert(large.all_sum() == 2);
    assert(large.range_sum(0, 3) == kLarge - 6);
    auto nodes = large.stored_nodes();
    assert(large.range_sum(1, 1) == 0 && large.stored_nodes() == nodes);

    sirkit::SparseAffineRangeSum<1> unit(kLength, 17);
    unit.range_affine(1, kLength - 1, -2, 9);
    assert(unit.range_sum(0, kLength) == 0 && unit.stored_nodes() == 0);

    std::mt19937_64 rng(6177766);
    for (int trial = 0; trial < 500; ++trial)
    {
        const auto n = static_cast<std::uint64_t>(trial % 117 + 1);
        const auto initial = static_cast<std::int64_t>(rng() % 1001) - 500;
        Tree candidate(n, initial);
        std::vector<std::uint32_t> values(n, Normalize(initial));
        for (int step = 0; step < 250; ++step)
        {
            const auto [left, right] = std::minmax({rng() % (n + 1), rng() % (n + 1)});
            if (step % 3 != 1)
            {
                const auto a = static_cast<std::int64_t>(rng() % 1001) - 500;
                const auto b = static_cast<std::int64_t>(rng() % 10'001) - 5000;
                const auto l = step % 11 == 0 ? 0 : left;
                const auto r = step % 11 == 0 ? n : right;
                candidate.range_affine(l, r, a, b);
                for (auto i = l; i < r; ++i)
                {
                    values[i] = static_cast<std::uint32_t>((std::uint64_t{Normalize(a)} * values[i] + Normalize(b)) % kMod);
                }
            }
            std::uint64_t sum = 0, all = 0;
            for (std::uint64_t i = 0; i < n; ++i)
            {
                all += values[i];
                if (left <= i && i < right)
                {
                    sum += values[i];
                }
            }
            const auto count = candidate.stored_nodes();
            assert(candidate.range_sum(left, right) == sum % kMod);
            assert(candidate.all_sum() == all % kMod);
            assert(candidate.range_sum(0, n) == all % kMod);
            assert(candidate.stored_nodes() == count);
        }
    }

    assert(sirkit::detail::NextSparseAffineNodeCapacity(4, 5, 5, 9) == 9);
    for (const auto used : {4U, 10U})
    {
        try
        {
            (void)sirkit::detail::NextSparseAffineNodeCapacity(used, 5, 6, 9);
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    Tree bounds(4);
    for (const auto &[left, right] : {std::pair{3, 1}, std::pair{0, 5}})
    {
        try
        {
            (void)bounds.range_sum(left, right);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
