// SPDX-License-Identifier: CC0-1.0
#include "Algorithm/Dyadic_Interval_Cover.cpp"

#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
void reference(int left, int right, int bound_left, int bound_right, std::vector<std::pair<int, int>> &out)
{
    if (bound_right <= left || right <= bound_left)
    {
        return;
    }
    if (left <= bound_left && bound_right <= right)
    {
        out.emplace_back(bound_left, bound_right);
        return;
    }
    const auto mid = (bound_left + bound_right) / 2;
    reference(left, right, bound_left, mid, out);
    reference(left, right, mid, bound_right, out);
}
}

int main()
{
    static_assert(sirkit::dyadic_interval_cover(-8, 8).size() == 2);
    static_assert(sirkit::dyadic_interval_cover(std::uint8_t{0}, std::uint8_t{255}).size() == 8);
    for (int left = -128; left <= 127; ++left)
    {
        for (int right = left; right <= 127; ++right)
        {
            std::vector<std::pair<int, int>> want;
            reference(left, right, -128, 0, want);
            reference(left, right, 0, 128, want);
            const auto got = sirkit::dyadic_interval_cover(static_cast<std::int8_t>(left), static_cast<std::int8_t>(right));
            if (std::vector<std::pair<int, int>>(got.begin(), got.end()) != want)
            {
                return 1;
            }
        }
    }
    constexpr auto kUnsignedMax = std::numeric_limits<std::uint64_t>::max();
    const auto wide = sirkit::dyadic_interval_cover(std::uint64_t{1}, kUnsignedMax);
    if (wide.size() != 126 || wide.front().first != 1 || wide.back().second != kUnsignedMax)
    {
        return 2;
    }
    auto previous = std::uint64_t{1};
    for (const auto &[left, right] : wide)
    {
        const auto width = right - left;
        if (left != previous || !std::has_single_bit(width) || left % width != 0)
        {
            return 3;
        }
        previous = right;
    }
    constexpr auto kSignedMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kSignedMax = std::numeric_limits<std::int64_t>::max();
    const auto signed_cover = sirkit::dyadic_interval_cover(kSignedMin, kSignedMax);
    if (signed_cover.size() != 64 || signed_cover.front() != std::pair{kSignedMin, std::int64_t{0}} ||
        signed_cover.back().second != kSignedMax)
    {
        return 4;
    }
    try
    {
        static_cast<void>(sirkit::dyadic_interval_cover(0, -1));
        return 5;
    }
    catch (const std::invalid_argument &)
    {
    }
}
