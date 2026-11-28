// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_RUN_LENGTH_Z_DEFINED
#include "../../../String/Run_Length_Z.cpp"
#endif
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
void check(std::span<const sirkit::RunLengthZ::Run> runs)
{
    std::string text;
    for (const auto [byte, count] : runs)
    {
        text.append(static_cast<std::size_t>(count), static_cast<char>(byte));
    }
    const sirkit::RunLengthZ index(runs);
    assert(index.size() == text.size());
    std::uint64_t sum = 0;
    for (std::size_t i = 0; i <= text.size(); ++i)
    {
        std::size_t count = 0;
        while (i + count < text.size() && text[i + count] == text[count])
        {
            ++count;
        }
        assert(index.prefix_at(i) == count);
        sum += count;
    }
    for (const auto modulus : std::array<std::uint32_t, 4>{1, 7, 1'000'000'007, 4'294'967'295U})
    {
        assert(index.sum_prefix_lengths(modulus) == sum % modulus);
    }
}

template<class Exception, class Action>
void check_error(Action run)
{
    bool threw = false;
    try
    {
        run();
    }
    catch (const Exception &)
    {
        threw = true;
    }
    assert(threw);
}
} // namespace

int main()
{
    using Run = sirkit::RunLengthZ::Run;
    check({});
    check(std::array{Run{'a', 0}, Run{'b', 0}});
    check(std::array{Run{'a', 3}, Run{'b', 0}, Run{'a', 4}});
    check(std::array{Run{'a', 2}, Run{'b', 2}, Run{'a', 3}, Run{'b', 4}, Run{'a', 5}});
    check(std::array{Run{'a', 4}, Run{'b', 2}, Run{'a', 3}, Run{'c', 4}, Run{'a', 5}, Run{'d', 2}});
    check(std::array{Run{255, 5}, Run{0, 3}, Run{255, 7}, Run{0, 1}, Run{255, 2}});

    std::mt19937 random(731088);
    for (int trial = 0; trial < 1400; ++trial)
    {
        std::vector<Run> runs(random() % 37);
        const auto alphabet = random() % 6 + 1;
        for (auto &[byte, count] : runs)
        {
            byte = static_cast<std::uint8_t>(random() % alphabet);
            count = random() % 8;
        }
        check(runs);
    }

    constexpr auto kMaximum = std::numeric_limits<std::uint64_t>::max();
    const sirkit::RunLengthZ maximum(std::array{Run{'a', kMaximum}});
    assert(maximum.size() == kMaximum && maximum.prefix_at(kMaximum) == 0);
    assert(maximum.prefix_at(0) == kMaximum && maximum.prefix_at(kMaximum - 1) == 1);
    assert(maximum.sum_prefix_lengths(97) == 84);
    assert(maximum.sum_prefix_lengths() == 348'644'138);
    assert(maximum.sum_prefix_lengths(std::numeric_limits<std::uint32_t>::max()) == 0);
    check_error<std::length_error>([&] { (void)sirkit::RunLengthZ(std::array{Run{'x', kMaximum}, Run{'y', 1}}); });
    check_error<std::invalid_argument>([&] { (void)maximum.sum_prefix_lengths(0); });
#ifndef SKIP_RUNTIME_CHECKS
    const sirkit::RunLengthZ empty({});
    check_error<std::out_of_range>([&] { (void)empty.prefix_at(1); });
#endif
}
