// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Nondecreasing_Subsequence.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
int brute(const std::vector<int> &symbols, int left, int right, int low, int high)
{
    std::vector<int> lengths(static_cast<std::size_t>(right - left));
    int best = 0;
    for (int i = left; i < right; ++i)
    {
        const auto symbol = symbols[static_cast<std::size_t>(i)];
        if (symbol < low || high < symbol)
        {
            continue;
        }
        int next = 1;
        for (int j = left; j < i; ++j)
        {
            if (symbols[static_cast<std::size_t>(j)] <= symbol)
            {
                next = std::max(next, lengths[static_cast<std::size_t>(j - left)] + 1);
            }
        }
        lengths[static_cast<std::size_t>(i - left)] = next;
        best = std::max(best, next);
    }
    return best;
}

#ifndef SKIP_RUNTIME_CHECKS
template <class Exception, class Function>
void expect_exception(Function action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
#endif
}

int main()
{
    sirkit::RangeNondecreasingSubsequence<1> empty;
    assert(empty.empty() && empty.size() == 0 && empty.all() == 0);
    assert(empty.longest(0, 0) == 0 && empty.longest(0, 0, 0, 0) == 0);
    const std::array zeros{0, 0, 0, 0, 0};
    sirkit::RangeNondecreasingSubsequence<1> one_symbol(zeros);
    one_symbol.set(3, 0);
    assert(!one_symbol.empty() && one_symbol.all() == 5 && one_symbol.replacement_count(1, 4) == 0);
    const std::array different{4, 2, 0, 3, 3, 4};
    sirkit::RangeNondecreasingSubsequence<5> typed(different);
    auto copy = typed;
    copy.set(0, 0);
    assert(copy.all() == 5 && typed.all() == 4);
    const auto moved = std::move(copy);
    assert(copy.empty() && moved.longest(0, 6, 2, 3) == 3);
    assert(typed.longest(0, 5, 1, 1) == 0);

    std::mt19937 gen(24728);
    for (int round = 0; round < 180; ++round)
    {
        const auto n = static_cast<int>(gen() % 33 + 1);
        std::vector<int> symbols(static_cast<std::size_t>(n));
        for (auto &value : symbols)
        {
            value = static_cast<int>(gen() % 4);
        }
        sirkit::RangeNondecreasingSubsequence<4> range(symbols);
        assert(range.size() == n);
        for (int step = 0; step < 120; ++step)
        {
            const auto p = static_cast<int>(gen() % static_cast<unsigned>(n));
            const auto symbol = static_cast<int>(gen() % 4);
            range.set(p, symbol);
            symbols[static_cast<std::size_t>(p)] = symbol;

            int left = static_cast<int>(gen() % static_cast<unsigned>(n + 1));
            int right = static_cast<int>(gen() % static_cast<unsigned>(n + 1));
            if (right < left)
            {
                std::swap(left, right);
            }
            int low = static_cast<int>(gen() % 4);
            int high = static_cast<int>(gen() % 4);
            if (low > high)
            {
                std::swap(low, high);
            }
            const auto best = brute(symbols, left, right, 0, 3);
            assert(range.longest(left, right) == best);
            assert(range.replacement_count(left, right) == right - left - best);
            assert(range.longest(left, right, low, high) == brute(symbols, left, right, low, high));
            assert(range.all() == brute(symbols, 0, n, 0, 3));
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::out_of_range>([&]() { typed.set(0, -1); });
    expect_exception<std::out_of_range>([&]() { typed.set(0, 5); });
    expect_exception<std::out_of_range>([&]() { typed.set(-1, 0); });
    expect_exception<std::out_of_range>([&]() { typed.set(6, 0); });
    expect_exception<std::out_of_range>([&]() { static_cast<void>(typed.longest(0, 0, -1, 0)); });
    expect_exception<std::out_of_range>([&]() { static_cast<void>(typed.longest(0, 0, 0, 5)); });
    expect_exception<std::invalid_argument>([&]() { static_cast<void>(typed.longest(0, 0, 4, 3)); });
    expect_exception<std::out_of_range>([&]() { static_cast<void>(typed.replacement_count(1, 0)); });
    expect_exception<std::out_of_range>([&]() { static_cast<void>(typed.longest(-1, 0)); });
    expect_exception<std::out_of_range>([&]() { static_cast<void>(typed.longest(1, 7)); });
    expect_exception<std::out_of_range>([]()
    {
        const std::array invalid{0, 4};
        const sirkit::RangeNondecreasingSubsequence<4> constructs(invalid);
    });
    assert(typed.all() == 4);
#endif
}
