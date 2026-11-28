// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Modular_Shift_Sums.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
void Verify(std::uint32_t limit, const std::vector<sirkit::ModularShiftTerm> &terms)
{
    const auto sums = sirkit::modular_shift_sums(limit, terms);
    assert(sums.size() == std::size_t{limit} + 1);
    for (std::uint64_t x = 0; x <= limit; ++x)
    {
        std::uint64_t expected = 0;
        for (const auto &[offset, modulus] : terms)
        {
            const auto reduced = offset % modulus;
            const auto residue = (reduced + static_cast<std::int64_t>(x)) % modulus;
            expected += static_cast<std::uint64_t>(residue < 0 ? residue + modulus : residue);
        }
        assert(sums[static_cast<std::size_t>(x)] == expected);
    }
}
} // namespace

int main()
{
    constexpr auto kMax = std::numeric_limits<std::uint32_t>::max();
    constexpr auto kHigh = std::numeric_limits<std::int64_t>::max();
    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    Verify(0, {});
    Verify(12, {});
    Verify(3, {{1, 3}, {2, 5}});
    Verify(13, {{0, 3}, {1, 3}, {2, 3}});
    Verify(19, {{0, 2}, {2, 2}, {1, 2}, {-1, 2}});
    Verify(19, {{0, 2}, {2, 2}, {1, 2}});
    Verify(20, {{0, 3}, {1, 3}, {-1, 4}});
    Verify(100, {{0, 17}, {3, 19}, {4, 19}, {-1, 1}});
    Verify(0, {{kLow, kMax}, {kHigh, kMax - 1}, {0, 2}, {1, 2}});
    Verify(13, {{kLow, kMax}, {kHigh, kMax - 1}, {0, 2}, {1, 2}});

    std::mt19937 random(563012);
    for (int trial = 0; trial < 950; ++trial)
    {
        const auto limit = static_cast<std::uint32_t>(random() % 320);
        std::vector<sirkit::ModularShiftTerm> terms(random() % 155);
        for (auto &[offset, modulus] : terms)
        {
            modulus = static_cast<std::uint32_t>(random() % (trial % 7 == 0 ? 9 : 180) + 1);
            offset = static_cast<std::int64_t>(random() % 1001) - 500;
        }
        Verify(limit, terms);
    }

    bool rejected = false;
    try
    {
        static_cast<void>(sirkit::modular_shift_sums(1, std::vector<sirkit::ModularShiftTerm>{{-1, 0}}));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
}
