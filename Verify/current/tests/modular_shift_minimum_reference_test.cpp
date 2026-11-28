// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Modular_Shift_Minimum.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::uint64_t Simulate(std::uint32_t count, std::uint32_t modulus, std::int64_t step,
                       const std::vector<std::int64_t> &offsets, std::int64_t start)
{
    auto phase = start % modulus;
    const auto slope = step % modulus;
    std::uint64_t answer = 0;
    for (std::uint32_t i = 0; i < count; ++i)
    {
        std::uint32_t best = modulus;
        for (const auto offset : offsets)
        {
            auto residue = (phase + offset % modulus) % modulus;
            if (residue < 0)
            {
                residue += modulus;
            }
            best = std::min(best, static_cast<std::uint32_t>(residue));
        }
        answer += best;
        phase = (phase + slope) % modulus;
    }
    return answer;
}
} // namespace

int main()
{
    using sirkit::sum_minimum_modular_shifts;
    const std::vector<std::int64_t> small{1, 3};
    assert(sum_minimum_modular_shifts(0, 5, 3, small) == 0);
    assert(sum_minimum_modular_shifts(3, 5, 3, small) == 4);
    assert(sum_minimum_modular_shifts(1, 5, 3, small, -2) == 1);
    assert(sum_minimum_modular_shifts(7, 5, 10, small, -2) == 7);
    assert(sum_minimum_modular_shifts(7, 1, 3, small, -2) == 0);
    const std::vector<std::int64_t> congruent{-10, 5, 0, 80};
    assert(sum_minimum_modular_shifts(10, 5, -2, congruent) == 20);

    constexpr auto kMax = std::numeric_limits<std::uint32_t>::max();
    constexpr auto kHigh = std::numeric_limits<std::int64_t>::max();
    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    const std::vector<std::int64_t> extreme{kLow};
    assert(sum_minimum_modular_shifts(kMax, kMax, -2, extreme, kHigh) == std::uint64_t{kMax} * (kMax - 1) / 2);
    const std::vector<std::int64_t> wrapped{0, -1, kMax};
    assert(sum_minimum_modular_shifts(kMax, kMax, -1, wrapped, kLow) == std::uint64_t{kMax - 1} * (kMax - 2) / 2);
    const std::vector<std::int64_t> largest{-1};
    assert(sum_minimum_modular_shifts(kMax, kMax, kMax, largest) == std::uint64_t{kMax} * (kMax - 1));

    std::mt19937 random(110294);
    for (int trial = 0; trial < 1200; ++trial)
    {
        const auto count = static_cast<std::uint32_t>(random() % 95);
        const auto modulus = trial % 11 == 0 ? kMax : static_cast<std::uint32_t>(random() % 170 + 1);
        const auto step = trial % 5 == 0 ? kLow : static_cast<std::int64_t>(random() % 1001) - 500;
        const auto start = trial % 7 == 0 ? kHigh : static_cast<std::int64_t>(random() % 10001) - 5000;
        std::vector<std::int64_t> offsets(random() % 22 + 1);
        for (auto &offset : offsets)
        {
            offset = static_cast<std::int64_t>(random() % 2001) - 1000;
        }
        assert(sum_minimum_modular_shifts(count, modulus, step, offsets, start) == Simulate(count, modulus, step, offsets, start));
    }

    bool invalid_modulus = false;
    try
    {
        static_cast<void>(sum_minimum_modular_shifts(1, 0, 0, small));
    }
    catch (const std::invalid_argument &)
    {
        invalid_modulus = true;
    }
    assert(invalid_modulus);
    bool invalid_offsets = false;
    try
    {
        static_cast<void>(sum_minimum_modular_shifts(1, 5, 0, {}));
    }
    catch (const std::invalid_argument &)
    {
        invalid_offsets = true;
    }
    assert(invalid_offsets);
}
