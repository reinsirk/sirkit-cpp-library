// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Sort_Aggregation.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Affine
{
    std::uint32_t a, b;
    bool operator==(const Affine &) const noexcept = default;
};

constexpr Affine Compose(Affine left, Affine right) noexcept
{
    return {
        static_cast<std::uint32_t>(std::uint64_t{right.a} * left.a % kMod),
        static_cast<std::uint32_t>((std::uint64_t{right.a} * left.b + right.b) % kMod),
    };
}

constexpr Affine Identity() noexcept
{
    return {1, 0};
}

using Data = sirkit::RangeSortAggregation<Affine, Compose, Identity>;

struct Element
{
    std::int32_t key;
    Affine f;
    int id;
};

void SortReference(std::vector<Element> &values, int left, int right, bool descending)
{
    std::sort(values.begin() + left, values.begin() + right, [descending](const Element &a, const Element &b)
              {
                  if (descending)
                  {
                      return std::tie(a.key, a.id) > std::tie(b.key, b.id);
                  }
                  return std::tie(a.key, a.id) < std::tie(b.key, b.id);
              });
}

Affine FoldReference(const std::vector<Element> &values, int left, int right)
{
    std::uint64_t zero = 0, one = 1;
    for (int i = left; i < right; ++i)
    {
        zero = (values[i].f.a * zero + values[i].f.b) % kMod;
        one = (values[i].f.a * one + values[i].f.b) % kMod;
    }
    return {static_cast<std::uint32_t>((one + kMod - zero) % kMod), static_cast<std::uint32_t>(zero)};
}

void CheckAll(const Data &data, const std::vector<Element> &reference)
{
    const int n = static_cast<int>(reference.size());
    assert(data.size() == n);
    assert(data.all_prod() == FoldReference(reference, 0, n));
    for (int i = 0; i < n; ++i)
    {
        const auto item = data.get(i);
        assert(item.key == reference[i].key && item.value == reference[i].f);
    }
    for (int left = 0; left <= n; ++left)
    {
        for (int right = left; right <= n; ++right)
        {
            assert(data.prod(left, right) == FoldReference(reference, left, right));
        }
    }
}
}

int main()
{
    Data empty;
    assert(empty.size() == 0 && empty.all_prod() == Identity() && empty.prod(0, 0) == Identity());
    empty.sort(0, 0, true);

    constexpr auto kMin = std::numeric_limits<std::int32_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int32_t>::max();
    std::vector<Element> expected{{kMax, {1, 1}, 0},
                                  {7, {2, 1}, 1},
                                  {kMin, {1, 4}, 2},
                                  {7, {5, 7}, 3},
                                  {6, {0, 2}, 4},
                                  {7, {8, 9}, 5},
                                  {kMin, {0, 6}, 6},
                                  {0, {9, 0}, 7}};
    std::vector<Data::Item> input;
    for (const auto &entry : expected)
    {
        input.push_back({entry.key, entry.f});
    }

    Data original(input);
    Data data(original);
    data.sort(0, 8, true);
    SortReference(expected, 0, 8, true);
    CheckAll(data, expected);
    data.sort(1, 7);
    SortReference(expected, 1, 7, false);
    data.sort(2, 3, true);
    data.set(4, 7, {17, 19});
    expected[4].key = 7;
    expected[4].f = {17, 19};
    data.sort(0, 8);
    SortReference(expected, 0, 8, false);
    CheckAll(data, expected);
    assert(original.get(0).key == kMax);

    original = data;
    const auto &copy_alias = original;
    original = copy_alias;
    CheckAll(original, expected);
    Data moved(std::move(original));
    assert(original.size() == 0 && original.prod(0, 0) == Identity());
    auto &move_alias = moved;
    moved = std::move(move_alias);
    CheckAll(moved, expected);
    original = std::move(moved);
    assert(moved.size() == 0 && moved.all_prod() == Identity());
    CheckAll(original, expected);

    std::mt19937 rng(281736);
    for (int trial = 0; trial < 450; ++trial)
    {
        const int n = trial % 76 + 1;
        std::vector<Element> model;
        std::vector<Data::Item> start;
        const auto key = [&]()
        {
            const auto type = rng() % 9;
            if (type == 0)
            {
                return kMin;
            }
            if (type == 1)
            {
                return kMax;
            }
            return static_cast<std::int32_t>(rng() % 41) - 20;
        };
        const auto value = [&]()
        {
            return Affine{static_cast<std::uint32_t>(rng() % 100), static_cast<std::uint32_t>(rng() % 100)};
        };
        for (int i = 0; i < n; ++i)
        {
            const Element item{key(), value(), i};
            model.push_back(item);
            start.push_back({item.key, item.f});
        }

        Data candidate(start);
        for (int i = 0; i < 230; ++i)
        {
            const auto [left, right] = std::minmax({static_cast<int>(rng() % (n + 1)), static_cast<int>(rng() % (n + 1))});
            if (i % 3 == 0)
            {
                const int p = static_cast<int>(rng() % n);
                const auto new_key = key();
                const auto new_value = value();
                candidate.set(p, new_key, new_value);
                model[p].key = new_key;
                model[p].f = new_value;
            }
            else
            {
                const bool descending = rng() % 2 != 0;
                candidate.sort(left, right, descending);
                SortReference(model, left, right, descending);
            }

            assert(candidate.prod(left, right) == FoldReference(model, left, right));
            assert(candidate.all_prod() == FoldReference(model, 0, n));
            const int p = static_cast<int>(rng() % n);
            const auto item = candidate.get(p);
            assert(item.key == model[p].key && item.value == model[p].f);
        }
        CheckAll(candidate, model);
    }

    assert(sirkit::detail::CheckedRangeSortAggregationBase(7, 8) == 8);
    for (const auto &[size, limit] : {
             std::pair{std::size_t{9}, std::size_t{8}},
             std::pair{static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1, std::size_t{8}},
         })
    {
        try
        {
            (void)sirkit::detail::CheckedRangeSortAggregationBase(size, limit);
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (int index : {-1, 8})
    {
        try
        {
            (void)data.get(index);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    for (const auto &[left, right] : {std::pair{-1, 4}, std::pair{5, 3}, std::pair{0, 9}})
    {
        try
        {
            (void)data.prod(left, right);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
