// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Assign_Range_Product.cpp"

#include <algorithm>
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
using Index = std::uint64_t;
constexpr Index kMod = 998244353;
constexpr Index kLargest = std::numeric_limits<Index>::max();

struct Affine
{
    Index a, b;

    constexpr Affine(Index multiplier, Index offset) noexcept
        : a(multiplier), b(offset)
    {
    }
    Affine() = delete;
    bool operator==(const Affine &) const = delete;
};

constexpr Affine Compose(const Affine &left, const Affine &right) noexcept
{
    return {right.a * left.a % kMod, (right.a * left.b + right.b) % kMod};
}

constexpr Affine Identity() noexcept
{
    return {1, 0};
}

bool Same(const Affine &left, const Affine &right)
{
    return left.a == right.a && left.b == right.b;
}

using Array = sirkit::RangeAssignRangeProduct<Affine, Compose, Identity>;

template <class Exception, class F>
void ExpectThrow(F action)
{
    bool thrown = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}

void CheckConstructor()
{
    Array a;
    assert(a.size() == 0 && a.stored_ranges() == 0);
    assert(Same(a.prod(0, 0), Identity()));
    assert(Same(a.all_prod(), Identity()));
    a.assign(0, 0, {0, 0});

    [[maybe_unused]] const Array b(0, {0, 1});
    [[maybe_unused]] const Array c(std::span<const Affine>{});
    Array d(100);
    assert(d.stored_ranges() == 1);
    assert(Same(d.get(99), Identity()));
    assert(Same(d.all_prod(), Identity()));
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)d.get(101);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       d.assign(10, 9, {1, 2});
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)d.prod(99, 101);
                                   });
#endif

    using sirkit::detail::NextRangeAssignProductCapacity;
    assert(NextRangeAssignProductCapacity(3, 5, 2, 8) == 5);
    assert(NextRangeAssignProductCapacity(5, 5, 2, 8) == 8);
    assert(NextRangeAssignProductCapacity(3, 3, 1, 10) == 6);
    assert(NextRangeAssignProductCapacity(0, 0, 2, 5) == 2);
    ExpectThrow<std::length_error>([]
                                   {
                                       (void)NextRangeAssignProductCapacity(5, 5, 4, 8);
                                   });
}

void CheckHugeLength()
{
    Array a(kLargest, {1, 13});
    const auto total = (kLargest % kMod) * 13 % kMod;
    assert(Same(a.prod(0, kLargest), {1, total}));
    assert(Same(a.prod(kLargest - 1, kLargest), {1, 13}));
    a.assign(kLargest - 2, kLargest - 1, {2, 1});
    assert(a.stored_ranges() == 3);
    assert(Same(a.prod(kLargest - 3, kLargest), {2, 40}));
    assert(Same(a.get(kLargest - 1), {1, 13}));
    assert(Same(a.get(kLargest - 2), {2, 1}));

    Array copy = a;
    a.assign(0, kLargest, {1, 4});
    assert(a.stored_ranges() == 1);
    assert(Same(a.all_prod(), {1, (kLargest % kMod) * 4 % kMod}));
    assert(Same(copy.get(kLargest - 2), {2, 1}));
    Array moved = std::move(copy);
    assert(copy.size() == 0 && copy.stored_ranges() == 0);
    assert(Same(moved.prod(kLargest - 3, kLargest), {2, 40}));
    copy = moved;
    moved = std::move(a);
    assert(a.size() == 0 && Same(a.all_prod(), Identity()));
    assert(Same(moved.get(kLargest - 1), {1, 4}));
    assert(Same(copy.get(kLargest - 2), {2, 1}));
    for (auto i = 0; i < 2000; ++i)
    {
        moved.assign(0, kLargest, {1, static_cast<Index>(i)});
        assert(moved.stored_ranges() == 1);
        assert(Same(moved.all_prod(), {1, (kLargest % kMod) * i % kMod}));
    }
}

Affine Evaluate(const std::vector<Affine> &values, std::size_t left, std::size_t right)
{
    Index zero = 0, one = 1;
    for (auto i = left; i < right; ++i)
    {
        zero = (values[i].a * zero + values[i].b) % kMod;
        one = (values[i].a * one + values[i].b) % kMod;
    }
    return {(one + kMod - zero) % kMod, zero};
}

void CheckRandom()
{
    std::mt19937_64 random(0xd91237d476c2350cULL);
    for (std::size_t n : {1, 2, 3, 16, 31, 80, 381})
    {
        std::vector<Affine> reference;
        for (auto i = std::size_t{0}; i < n; ++i)
        {
            reference.emplace_back(random() % kMod, random() % kMod);
        }
        Array actual(reference);
        assert(actual.size() == n && actual.stored_ranges() == n);
        for (auto step = 0; step < 7500; ++step)
        {
            auto left = random() % (n + 1);
            auto right = random() % (n + 1);
            if (right < left)
            {
                std::swap(right, left);
            }
            if (step % 3 != 2)
            {
                const Affine value{random() % kMod, random() % kMod};
                actual.assign(left, right, value);
                std::fill(reference.begin() + left, reference.begin() + right, value);
            }
            else
            {
                assert(Same(actual.prod(left, right), Evaluate(reference, left, right)));
            }
            const auto position = random() % n;
            assert(Same(actual.get(position), reference[position]));
            if (step % 3 == 0)
            {
                assert(Same(actual.all_prod(), Evaluate(reference, 0, n)));
            }
            assert(actual.stored_ranges() > 0 && actual.stored_ranges() <= n);
        }
    }
}

void CheckAlternatingBorders()
{
    constexpr std::size_t n = 8192;
    Array array(n);
    std::vector<Affine> reference(n, Identity());
    for (auto repetition = 0; repetition < 3; ++repetition)
    {
        for (std::size_t i = 0; i < n; ++i)
        {
            const auto position = i % 2 == 0 ? i / 2 : n - 1 - i / 2;
            const Affine value{i + 1, position + 7};
            array.assign(position, position + 1, value);
            reference[position] = value;
        }
        assert(array.stored_ranges() == n);
        for (std::size_t i = 1; i < n / 2; i += 11)
        {
            assert(Same(array.prod(i, n - i), Evaluate(reference, i, n - i)));
        }
        array.assign(0, n, {1, 2});
        assert(array.stored_ranges() == 1);
        assert(Same(array.all_prod(), {1, 2 * n}));
    }
}
}

int main()
{
    CheckConstructor();
    CheckHugeLength();
    CheckRandom();
    CheckAlternatingBorders();
}
