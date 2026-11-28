// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Repeated_Subsequence.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <utility>

namespace
{
int allocations_before_failure = -1;
}

void *operator new(std::size_t size)
{
    if (allocations_before_failure >= 0 && allocations_before_failure-- == 0)
    {
        throw std::bad_alloc();
    }
    if (auto *const buffer = std::malloc(size == 0 ? 1 : size))
    {
        return buffer;
    }
    throw std::bad_alloc();
}

void operator delete(void *pointer) noexcept
{
    std::free(pointer);
}

void operator delete(void *pointer, std::size_t) noexcept
{
    std::free(pointer);
}

using sirkit::RepeatedSubsequenceIndex;

bool brute(std::string_view expanded_cycle, std::string_view target, std::uint64_t repeats)
{
    std::string expanded_target;
    for (const char byte : target)
    {
        expanded_target.append(static_cast<std::size_t>(repeats), byte);
    }
    std::size_t consumed = 0;
    for (const char byte : expanded_cycle)
    {
        if (consumed == expanded_target.size())
        {
            break;
        }
        if (byte == expanded_target[consumed])
        {
            ++consumed;
        }
    }
    return consumed == expanded_target.size();
}

void boundary_cases()
{
    constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
    const RepeatedSubsequenceIndex empty("");
    assert(empty.contains_expanded(0, "", maximum));
    assert(empty.contains_expanded(maximum, "missing", 0));
    assert(!empty.contains_expanded(maximum, "missing", 1));
    assert(empty.max_repetition(maximum, "") == maximum);
    assert(empty.max_repetition(maximum, "x") == 0);

    const RepeatedSubsequenceIndex single("a");
    assert(single.contains_expanded(maximum, "a", maximum));
    assert(!single.contains_expanded(maximum, "aa", maximum));
    assert(single.max_repetition(maximum, "a") == maximum);
    assert(single.max_repetition(maximum, "aa") == maximum / 2);
    assert(single.max_repetition(0, "a") == 0);
    assert(single.max_repetition(maximum, "a", 0) == 0);
    assert(!single.contains_expanded(0, "a", 1));
    assert(single.max_repetition(0, "", 19) == 19);

    const RepeatedSubsequenceIndex alternating("ab");
    assert(alternating.max_repetition(maximum, "ba") == maximum / 2);
    assert(alternating.max_repetition(maximum, "ab") == maximum / 2 + 1);
    assert(alternating.max_repetition(maximum, "ab", 100) == 100);
    assert(!alternating.contains_expanded(1, "ba", 1));
    assert(alternating.contains_expanded(2, "ba", 1));
    const RepeatedSubsequenceIndex saturating("aaa");
    assert(saturating.max_repetition(maximum, "a") == maximum);
    assert(saturating.contains_expanded(maximum, "aaa", maximum));
    assert(!saturating.contains_expanded(maximum, "aaaa", maximum));

    const std::string binary{static_cast<char>(255), '\0', static_cast<char>(255)};
    const RepeatedSubsequenceIndex bytes(binary);
    const std::string reversed{'\0', static_cast<char>(255)};
    assert(bytes.contains_expanded(3, reversed, 2));
    assert(bytes.max_repetition(3, reversed) == 2);
}

void ownership_cases()
{
    RepeatedSubsequenceIndex original("aaa");
    const RepeatedSubsequenceIndex other("bbbbccccc");
    allocations_before_failure = 1;
    bool rejected = false;
    try
    {
        original = other;
    }
    catch (const std::bad_alloc &)
    {
        rejected = true;
    }
    allocations_before_failure = -1;
    assert(rejected && original.max_repetition(1, "a") == 3 && original.max_repetition(1, "b") == 0);

    original = other;
    RepeatedSubsequenceIndex moved(std::move(original));
    assert(moved.max_repetition(1, "b") == 4);
    assert(!original.contains_expanded(1, "b", 1));
    original = std::move(moved);
    assert(original.max_repetition(1, "c") == 5 && moved.max_repetition(1, "c") == 0);
}

void randomized_cases()
{
    constexpr std::array<char, 5> alphabet{'a', 'b', 'c', '\0', static_cast<char>(255)};
    std::mt19937 random(0xc7732a);
    for (int test = 0; test < 900; ++test)
    {
        std::string cycle(random() % 12, '\0'), target(random() % 10, '\0');
        for (auto &byte : cycle)
        {
            byte = alphabet[random() % alphabet.size()];
        }
        for (auto &byte : target)
        {
            byte = alphabet[random() % alphabet.size()];
        }
        const std::uint64_t copies = random() % 13;
        const std::uint64_t limit = random() % 21;
        std::string expanded;
        for (std::uint64_t i = 0; i < copies; ++i)
        {
            expanded += cycle;
        }
        const RepeatedSubsequenceIndex index(cycle);
        std::uint64_t expected = 0;
        for (std::uint64_t repeats = 0; repeats <= limit; ++repeats)
        {
            const bool fits = brute(expanded, target, repeats);
            assert(index.contains_expanded(copies, target, repeats) == fits);
            if (fits)
            {
                expected = repeats;
            }
        }
        assert(index.max_repetition(copies, target, limit) == expected);
    }
}

int main()
{
    boundary_cases();
    randomized_cases();
    ownership_cases();
}
