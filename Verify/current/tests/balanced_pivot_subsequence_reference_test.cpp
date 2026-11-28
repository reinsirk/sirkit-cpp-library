// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Balanced_Pivot_Subsequence.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <optional>
#include <random>
#include <stdexcept>
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

using Result = sirkit::BalancedPivotSubsequenceResult;
using Index = sirkit::BalancedPivotSubsequenceIndex;

std::optional<Result> brute(std::string_view text, std::size_t left, std::size_t right,
                            std::array<char, 3> pattern)
{
    std::optional<Result> result;
    for (std::size_t pivot = left; pivot < right; ++pivot)
    {
        if (text[pivot] != pattern[1])
        {
            continue;
        }
        const auto first = text.substr(left, pivot - left);
        const auto last = text.substr(pivot + 1, right - pivot - 1);
        const auto before = std::ranges::count(first, pattern[0]);
        const auto after = std::ranges::count(last, pattern[2]);
        const auto pairs = static_cast<std::size_t>(std::min(before, after));
        if (!result || pairs > result->pairs)
        {
            result = Result{pivot, pairs};
        }
    }
    return result;
}

void expect_result(const std::optional<Result> &result, std::size_t pivot, std::size_t pairs)
{
    assert(result && result->pivot == pivot && result->pairs == pairs && result->length() == 2 * pairs + 1);
}

#ifndef SKIP_RUNTIME_CHECKS
void expect_bad_range(const Index &index, std::size_t left, std::size_t right)
{
    bool caught = false;
    try
    {
        static_cast<void>(index.query(left, right));
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
}
#endif

void boundary_cases()
{
    Index empty("", 'a', '/', 'b');
    assert(empty.size() == 0 && !empty.query(0, 0));
    const Index no_pivot("aaaabbb", 'a', '/', 'b');
    assert(!no_pivot.query(0, 7));
    const Index only_left("aaa///", 'a', '/', 'b');
    expect_result(only_left.query(0, 6), 3, 0);
    const Index only_right("///bbb", 'a', '/', 'b');
    expect_result(only_right.query(0, 6), 0, 0);
    const Index tie("aa////bb", 'a', '/', 'b');
    expect_result(tie.query(0, 8), 2, 2);
    expect_result(tie.query(3, 4), 3, 0);
    assert(!tie.query(3, 3));

    const Index common("aaaaa", 'a', 'a', 'a');
    expect_result(common.query(0, 5), 2, 2);
    expect_result(common.query(0, 4), 1, 1);
    const std::string binary{'\0', 'x', '\0', static_cast<char>(255)};
    const Index bytes(binary, '\0', 'x', static_cast<char>(255));
    expect_result(bytes.query(0, 4), 1, 1);

    Index movable = tie;
    const Index moved(std::move(movable));
    assert(moved.size() == tie.size());
    expect_result(moved.query(0, 8), 2, 2);
    assert(movable.size() == 0 && !movable.query(0, 0));
    movable = moved;
    expect_result(movable.query(0, 8), 2, 2);

#ifndef SKIP_RUNTIME_CHECKS
    expect_bad_range(empty, 0, 1);
    expect_bad_range(tie, 5, 4);
    expect_bad_range(tie, 0, 9);
#endif
}

void failed_copy_assignment()
{
    Index original("a/a/bb", 'a', '/', 'b');
    const Index other(std::string(1200, '/'), 'a', '/', 'b');
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

    assert(rejected && original.size() == 6);
    expect_result(original.query(0, 6), 3, 2);
}

void randomized_cases()
{
    constexpr std::array<char, 4> alphabet{'a', 'b', '/', '\0'};
    std::mt19937 random(0xd123a9);
    for (int test = 0; test < 600; ++test)
    {
        std::string text(random() % 23, '\0');
        for (auto &byte : text)
        {
            byte = alphabet[random() % alphabet.size()];
        }
        std::array<char, 3> pattern{};
        for (auto &byte : pattern)
        {
            byte = alphabet[random() % alphabet.size()];
        }
        const Index index(text, pattern[0], pattern[1], pattern[2]);
        assert(index.size() == text.size());
        for (std::size_t l = 0; l <= text.size(); ++l)
        {
            for (std::size_t r = l; r <= text.size(); ++r)
            {
                const auto expected = brute(text, l, r, pattern);
                const auto result = index.query(l, r);
                if (expected)
                {
                    expect_result(result, expected->pivot, expected->pairs);
                }
                else
                {
                    assert(!result);
                }
            }
        }
    }
}

int main()
{
    boundary_cases();
    randomized_cases();
    failed_copy_assignment();
}
