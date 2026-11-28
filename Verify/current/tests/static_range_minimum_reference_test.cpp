// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_STATIC_RANGE_MINIMUM_DEFINED
#include "../../../DataStructure/Static_Range_Minimum.cpp"
#endif
#ifndef SIRKIT_SPARSE_TABLE_DEFINED
#include "../../../DataStructure/Sparse_Table.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
struct Minimum
{
    int operator()(int a, int b) const
    {
        return std::min(a, b);
    }
};
} // namespace

int main()
{
    std::mt19937 random(944712);
    const sirkit::StaticRangeMinimum<int> empty({});
    assert(empty.size() == 0);
    for (std::size_t n : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 65, 101, 256, 777})
    {
        std::vector<int> values(n);
        for (auto &value : values)
        {
            value = static_cast<int>(random() % 31) - 15;
        }
        const sirkit::StaticRangeMinimum<int> index(values);
        const sirkit::SparseTable<int, Minimum> old(values, std::numeric_limits<int>::max());
        auto copied = index;
        copied = empty;
        assert(copied.size() == 0 && index.size() == n);
        copied = index;
        for (int trial = 0; trial < 1400; ++trial)
        {
            const auto left = random() % n;
            const auto right = left + 1 + random() % (n - left);
            const auto expected = std::min_element(values.begin() + static_cast<std::ptrdiff_t>(left),
                                                   values.begin() + static_cast<std::ptrdiff_t>(right));
            assert(copied.argmin(left, right) == static_cast<std::size_t>(expected - values.begin()));
            assert(copied.min(left, right) == *expected);
            assert(index.min(left, right) == old.prod(left, right));
        }
    }
    const sirkit::StaticRangeMinimum<bool> bits({true, false, false, true});
    assert(!bits.min(0, 4) && bits.argmin(0, 4) == 1);
    assert(bits.min(3, 4));
    const sirkit::StaticRangeMinimum<int, std::greater<>> maximum({1, 7, 7, -4, 6, 7, 7, 1});
    assert(maximum.min(0, 8) == 7 && maximum.argmin(0, 8) == 1);
    assert(maximum.argmin(2, 8) == 2);

    constexpr std::size_t kSize = 200'000;
    const sirkit::StaticRangeMinimum<int> ties(std::vector<int>(kSize, 7));
    for (std::size_t left = 0; left < kSize; left += 31)
    {
        assert(ties.argmin(left, kSize) == left);
        assert(ties.argmin(left, left + 1) == left);
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto right : {std::size_t{0}, kSize + 1})
    {
        bool thrown = false;
        try
        {
            (void)ties.argmin(0, right);
        }
        catch (const std::out_of_range &)
        {
            thrown = true;
        }
        assert(thrown);
    }
#endif
}
