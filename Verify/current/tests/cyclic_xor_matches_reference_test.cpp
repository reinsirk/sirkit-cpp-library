// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Cyclic_Xor_Matches.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

using Value = std::uint64_t;
using sirkit::cyclic_xor_matches;

void compare_with_brute(std::span<const Value> source, std::span<const Value> target)
{
    const auto actual = cyclic_xor_matches(source, target);
    std::size_t matches = 0;
    for (std::size_t shift = 0; shift < source.size(); ++shift)
    {
        const Value mask = source[shift] ^ target.front();
        bool works = true;
        for (std::size_t i = 0; i < source.size(); ++i)
        {
            works = works && (source[(i + shift) % source.size()] ^ mask) == target[i];
        }
        if (works)
        {
            assert(matches < actual.size());
            assert(actual[matches].shift == static_cast<int>(shift) && actual[matches].mask == mask);
            ++matches;
        }
    }
    assert(matches == actual.size());
}

template <typename Error, typename Operation>
void expect_error(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}

void edge_cases()
{
    assert(cyclic_xor_matches({}, {}).empty());
    constexpr Value all_bits = std::numeric_limits<Value>::max();
    const std::array<Value, 1> low{0}, high{all_bits};
    const auto one = cyclic_xor_matches(low, high);
    assert(one.size() == 1 && one[0].shift == 0 && one[0].mask == all_bits);

    const std::vector<Value> source(26, all_bits), target(26, 0);
    compare_with_brute(source, target);
    const std::array<Value, 4> alternating{Value{1} << 63, 5, Value{1} << 63, 5};
    const std::array<Value, 4> permuted{5, Value{1} << 63, 5, Value{1} << 63};
    compare_with_brute(alternating, permuted);
    const std::array<Value, 2> different{0, 1}, equal{0, 0};
    assert(cyclic_xor_matches(different, equal).empty());

    constexpr auto maximum = static_cast<std::size_t>(std::numeric_limits<int>::max() / 2);
    sirkit::detail::CheckCyclicXorInputLength(maximum);
    expect_error<std::length_error>([]
    {
        sirkit::detail::CheckCyclicXorInputLength(maximum + 1);
    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([&]
    {
        static_cast<void>(cyclic_xor_matches(low, {}));
    });
#endif
}

void randomized_cases()
{
    std::mt19937_64 random(0x4fc12d);
    for (int test = 0; test < 2100; ++test)
    {
        const std::size_t n = 1 + random() % 42;
        std::vector<Value> source(n), target(n);
        for (auto &value : source)
        {
            value = test % 3 == 0 ? random() % 7 : random();
        }
        if (test % 4 == 0)
        {
            for (auto &value : target)
            {
                value = random() % 12;
            }
        }
        else
        {
            const auto shift = random() % n;
            const auto mask = random();
            for (std::size_t i = 0; i < n; ++i)
            {
                target[i] = source[(i + shift) % n] ^ mask;
            }
        }
        compare_with_brute(source, target);
    }
}

int main()
{
    edge_cases();
    randomized_cases();
}
