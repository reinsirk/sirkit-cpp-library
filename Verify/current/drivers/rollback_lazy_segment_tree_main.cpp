// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{
constexpr unsigned kBits = 30;
constexpr std::uint32_t kMask = (std::uint32_t{1} << kBits) - 1;
struct Summary
{
    std::array<std::uint32_t, kBits> counts{};
    std::uint64_t sum = 0;
    std::uint32_t some = 0;
    std::uint32_t every = kMask;
};
struct BitUpdate
{
    std::uint32_t keep;
    std::uint32_t put;
};

Summary Identity()
{
    return {};
}

Summary Combine(const Summary &a, const Summary &b)
{
    Summary result;
    for (unsigned i = 0; i < kBits; ++i)
    {
        result.counts[i] = a.counts[i] + b.counts[i];
    }
    result.sum = a.sum + b.sum;
    result.some = a.some | b.some;
    result.every = a.every & b.every;
    return result;
}

Summary Map(BitUpdate f, Summary value, std::size_t length)
{
    // In f.put bits are forced to one; other cleared bits are forced to zero.
    std::uint32_t changed = (value.some & ~(f.keep | f.put)) | (f.put & ~value.every);
    while (changed != 0)
    {
        const unsigned bit = std::countr_zero(changed);
        const auto bit_mask = std::uint32_t{1} << bit;
        changed &= changed - 1;
        const auto count = (f.put & bit_mask) != 0 ? static_cast<std::uint32_t>(length) : 0;
        value.sum -= std::uint64_t(value.counts[bit]) << bit;
        value.sum += std::uint64_t(count) << bit;
        value.counts[bit] = count;
    }
    value.some = (value.some & f.keep) | f.put;
    value.every = (value.every & f.keep) | f.put;
    return value;
}

BitUpdate Compose(BitUpdate after, BitUpdate before)
{
    return {after.keep & before.keep, (before.put & after.keep) | after.put};
}

bool NoChange(BitUpdate f, const Summary &value, std::size_t)
{
    return (value.some & ~(f.keep | f.put)) == 0 && (f.put & ~value.every) == 0;
}

Summary Single(std::uint32_t value)
{
    Summary result;
    result.sum = result.some = result.every = value;
    for (unsigned i = 0; i < kBits; ++i)
    {
        result.counts[i] = (value >> i) & 1u;
    }
    return result;
}
}

int main()
{
    std::uint32_t n = 0, m = 0;
    verify_in >> n >> m;
    std::vector<Summary> values(n);
    for (auto &entry : values)
    {
        std::uint32_t value = 0;
        verify_in >> value;
        entry = Single(value);
    }
    std::array<std::vector<std::uint32_t>, 5> parameters;
    for (auto &row : parameters)
    {
        row.resize(m);
        for (auto &x : row)
        {
            verify_in >> x;
        }
    }
    sirkit::RollbackLazySegmentTree<Summary, BitUpdate, Combine, Identity, Map, Compose, NoChange> tree(values);
    const auto initial = tree.snapshot();
    std::uint32_t q = 0;
    verify_in >> q;
    for (std::uint32_t group = 1; group <= q; ++group)
    {
        std::uint32_t start = 0, updates = 0;
        verify_in >> start >> updates;
        std::uint32_t y = group;
        for (std::uint32_t j = 1; j <= updates; ++j)
        {
            const auto z = (start + j) % m;
            const auto index = [&](int row)
            {
                return std::clamp(parameters[row][z] ^ y, std::uint32_t{1}, n);
            };
            const auto a = index(0), b = index(1);
            const auto left = index(3), right = index(4);
            const auto mask = parameters[2][z] ^ y;
            const auto f = (z % 2) != 0 ? BitUpdate{kMask ^ mask, mask} : BitUpdate{mask, 0};
            tree.apply(std::min(a, b) - 1, std::max(a, b), f);
            y = static_cast<std::uint32_t>(tree.prod(std::min(left, right) - 1, std::max(left, right)).sum) & kMask;
        }
        verify_out << y << '\n';
        tree.rollback(initial);
    }
}
