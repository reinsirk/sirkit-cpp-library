// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_XOR_BASIS_INTERSECTION_DEFINED
#include "../../../Math/Xor_Basis_Intersection.cpp"
#endif
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <random>
#include <span>
#include <vector>

namespace
{
using Vectors = std::vector<std::uint64_t>;

// XOR部分集合をそのまま列挙する。最高位の消去を使わない。
Vectors span_values(std::span<const std::uint64_t> basis)
{
    Vectors span{0};
    for (const auto bitset : basis)
    {
        const auto old_size = span.size();
        for (std::size_t i = 0; i < old_size; ++i)
        {
            span.push_back(span[i] ^ bitset);
        }
    }
    std::ranges::sort(span);
    span.erase(std::unique(span.begin(), span.end()), span.end());
    return span;
}

void compare(const Vectors &left, const Vectors &right)
{
    const auto x = span_values(left);
    const auto y = span_values(right);
    Vectors expected;
    std::ranges::set_intersection(x, y, std::back_inserter(expected));
    const auto basis = sirkit::xor_basis_intersection(left, right);
    assert(basis.size() <= 10);
    const auto actual = span_values(basis);
    assert(actual.size() == (std::size_t{1} << basis.size()));
    assert(actual == expected);
}
} // namespace

int main(int argc, char **)
{
    constexpr std::uint64_t kHigh = std::uint64_t{1} << 63;
    compare({}, {});
    compare({1}, {});
    compare({}, {1});
    compare({0, 0}, {0, 1, 0});
    compare({1, 2}, {4, 8});
    compare({1, 2, 0, 3, 2}, {3, 3, 0, 3});
    compare({kHigh | 123, 123}, {kHigh, kHigh | 7, 42});
    compare({kHigh, 1, 2, 8}, {kHigh, 1, 3, 4, 32});

    std::mt19937_64 rng(3071227);
    for (std::size_t trial = 0; trial < 2400; ++trial)
    {
        Vectors left(rng() % 10), right(rng() % 10);
        const auto parameters = static_cast<unsigned>(1 + rng() % 9);
        const auto mask = (std::uint64_t{1} << parameters) - 1;
        for (auto &value : left)
        {
            value = rng() & mask;
        }
        for (auto &value : right)
        {
            value = rng() & mask;
        }
        compare(left, right);
        compare(right, left);
    }

    std::array<std::uint64_t, 64> complete{};
    for (std::size_t i = 0; i < complete.size(); ++i)
    {
        complete[i] = std::uint64_t{1} << i;
    }
    const auto common_full = sirkit::xor_basis_intersection(complete, complete);
    assert(std::ranges::equal(complete, common_full));
    const std::array<std::uint64_t, 3> narrow{kHigh | 1, kHigh | 4, 8};
    const auto from_full = sirkit::xor_basis_intersection(complete, narrow);
    assert(from_full.size() == narrow.size() && span_values(from_full) == span_values(narrow));

    if (argc > 1)
    {
        // 低位30本と13ビットずらした30本。各基底内で三角変換しても、共通部分は13..29ビット。
        constexpr std::size_t kOffset = 13;
        constexpr std::uint64_t kExpected = ((std::uint64_t{1} << 17) - 1) << kOffset;
        std::array<std::uint64_t, 30> left{}, right{};
        for (std::size_t trial = 0; trial < 100000; ++trial)
        {
            for (std::size_t j = 0; j < left.size(); ++j)
            {
                const auto bit = std::uint64_t{1} << j;
                left[j] = bit | (rng() & (bit - 1));
                right[j] = (bit | (rng() & (bit - 1))) << kOffset;
            }
            const auto intersection = sirkit::xor_basis_intersection(left, right);
            assert(intersection.size() == 17);
            std::uint64_t leading = 0;
            for (const auto vector : intersection)
            {
                assert((vector & ~kExpected) == 0);
                leading |= std::bit_floor(vector);
            }
            assert(leading == kExpected);
        }
    }
}
