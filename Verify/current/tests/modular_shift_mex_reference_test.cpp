// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Modular_Shift_Mex.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
std::uint64_t Simulate(std::uint32_t count, std::uint32_t modulus, std::int64_t step,
                       std::int64_t start, const std::vector<bool> &occupied)
{
    auto phase = start % modulus;
    const auto slope = step % modulus;
    std::uint64_t answer = 0;
    for (std::uint32_t i = 0; i < count; ++i)
    {
        for (std::uint32_t distance = 0; distance < modulus; ++distance)
        {
            auto value = (distance - phase) % modulus;
            if (value < 0)
            {
                value += modulus;
            }
            if (!occupied[static_cast<std::size_t>(value)])
            {
                break;
            }
            ++answer;
        }
        phase = (phase + slope) % modulus;
    }
    return answer;
}
} // namespace

int main()
{
    using sirkit::ModularShiftMex;
    ModularShiftMex sample(2, 3, 1);
    assert(sample.sum() == 0 && sample.size() == 0);
    assert(sample.insert(2));
    assert(sample.insert(0));
    assert(sample.sum() == 3 && sample.size() == 2);
    assert(!sample.insert(2));
    assert(sample.erase(0));
    assert(sample.insert(1));
    assert(sample.sum() == 1);
    assert(sample.insert(0));
    assert(sample.sum() == 6);
    assert(sample.erase(1));
    assert(sample.sum() == 3);
    assert(!sample.erase(1));

    ModularShiftMex copied(sample);
    assert(copied.erase(2));
    assert(copied.sum() == 1 && sample.sum() == 3);
    ModularShiftMex moved(std::move(copied));
    assert(moved.size() == 1 && moved.sum() == 1);
    assert(copied.size() == 0 && copied.insert(1));
    moved = sample;
    assert(moved.size() == 2 && moved.sum() == 3);
    auto *same = std::addressof(moved);
    moved = *same;
    moved = std::move(*same);
    assert(moved.size() == 2 && moved.sum() == 3);
    copied = std::move(moved);
    assert(copied.size() == 2 && copied.sum() == 3 && moved.size() == 0);
    assert(moved.insert(2));
    const auto previous_total = moved.sum();
    moved.clear();
    assert(moved.size() == 0 && moved.sum() == 0);
    assert(moved.insert(2) && moved.sum() == previous_total);

    constexpr auto kMax = std::numeric_limits<std::uint32_t>::max();
    constexpr auto kHigh = std::numeric_limits<std::int64_t>::max();
    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    ModularShiftMex large(kMax, kMax, -1, kLow);
    assert(large.insert(kMax - 1) && large.sum() == 1);
    assert(large.insert(0) && large.sum() == 3);
    assert(large.insert(1) && large.sum() == 6);
    assert(large.erase(0) && large.sum() == 2);
    assert(large.erase(kMax - 1) && large.sum() == 1);
    assert(large.erase(1) && large.sum() == 0);
    ModularShiftMex singleton(kMax, 1, kHigh, kLow);
    assert(singleton.insert(0) && singleton.sum() == kMax);
    assert(!singleton.insert(0));
    assert(singleton.erase(0) && singleton.sum() == 0);
    assert(!singleton.erase(0));

    std::mt19937 random(846310);
    for (int trial = 0; trial < 400; ++trial)
    {
        const auto modulus = static_cast<std::uint32_t>(random() % (trial % 2 == 0 ? 8 : 63) + 1);
        const auto count = static_cast<std::uint32_t>(random() % 80);
        const auto step = trial % 5 == 0 ? kLow : static_cast<std::int64_t>(random() % 1001) - 500;
        const auto start = trial % 7 == 0 ? kHigh : static_cast<std::int64_t>(random() % 10001) - 5000;
        ModularShiftMex values(count, modulus, step, start);
        std::vector<bool> occupied(modulus);
        for (int update = 0; update < 120; ++update)
        {
            const auto value = static_cast<std::uint32_t>(random() % modulus);
            const bool adding = random() % 2 == 0;
            const bool changed = static_cast<bool>(occupied[value]) != adding;
            assert((adding ? values.insert(value) : values.erase(value)) == changed);
            occupied[value] = adding;
            assert(values.size() == static_cast<std::uint32_t>(std::ranges::count(occupied, true)));
            assert(values.sum() == Simulate(count, modulus, step, start, occupied));
        }
    }

    bool invalid_modulus = false;
    try
    {
        const ModularShiftMex invalid(1, 0, 0);
    }
    catch (const std::invalid_argument &)
    {
        invalid_modulus = true;
    }
    assert(invalid_modulus);

    bool invalid_residue = false;
    try
    {
        sample.insert(3);
    }
    catch (const std::out_of_range &)
    {
        invalid_residue = true;
    }
    assert(invalid_residue && sample.sum() == 3);
}
