// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_ROLLBACK_LIS_DEFINED
#include "../../../Algorithm/Rollback_LIS.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr auto kNoAllocationLimit = std::numeric_limits<std::size_t>::max();
std::size_t remaining_allocations = kNoAllocationLimit;
}

#if defined(__GNUC__) || defined(__clang__)
#define SIRKIT_TEST_NOINLINE [[gnu::noinline]]
#else
#define SIRKIT_TEST_NOINLINE
#endif

SIRKIT_TEST_NOINLINE void *operator new(std::size_t size)
{
    if (remaining_allocations != kNoAllocationLimit)
    {
        if (remaining_allocations == 0)
        {
            throw std::bad_alloc();
        }
        --remaining_allocations;
    }
    if (void *memory = std::malloc(size == 0 ? 1 : size))
    {
        return memory;
    }
    throw std::bad_alloc();
}

SIRKIT_TEST_NOINLINE void operator delete(void *memory) noexcept
{
    std::free(memory);
}

SIRKIT_TEST_NOINLINE void operator delete(void *memory, std::size_t) noexcept
{
    ::operator delete(memory);
}

#undef SIRKIT_TEST_NOINLINE

namespace
{
template <bool Strict>
void compare(const sirkit::RollbackLIS<int, Strict> &lis, const std::vector<int> &values)
{
    std::vector<std::size_t> best(values.size(), 1);
    std::size_t longest = 0;
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        for (std::size_t j = 0; j < i; ++j)
        {
            if (Strict ? values[j] < values[i] : values[j] <= values[i])
            {
                best[i] = std::max(best[i], best[j] + 1);
            }
        }
        longest = std::max(longest, best[i]);
    }
    assert(lis.size() == values.size());
    assert(lis.empty() == values.empty());
    assert(lis.length() == longest);
    const auto indices = lis.indices();
    assert(indices.size() == longest);
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        assert(indices[i] < values.size());
        if (i != 0)
        {
            assert(indices[i - 1] < indices[i]);
            if constexpr (Strict)
            {
                assert(values[indices[i - 1]] < values[indices[i]]);
            }
            else
            {
                assert(values[indices[i - 1]] <= values[indices[i]]);
            }
        }
    }
}

template <bool Strict>
void randomized_history()
{
    sirkit::RollbackLIS<int, Strict> lis;
    std::vector<int> values;
    std::mt19937 random(80731);
    for (int step = 0; step < 5000; ++step)
    {
        const auto action = random() % 6;
        if (values.empty() || action < 4)
        {
            const auto next = static_cast<int>(random() % 15) - 7;
            lis.push_back(next);
            values.push_back(next);
        }
        else if (action == 4)
        {
            lis.pop_back();
            values.pop_back();
        }
        else
        {
            const auto retain = random() % (values.size() + 1);
            lis.rollback(retain);
            values.resize(retain);
        }
        compare(lis, values);
    }

    auto copy = lis;
    auto moved = std::move(copy);
    compare(moved, values);
    compare(copy, {});
    copy = moved;
    moved = std::move(lis);
    compare(copy, values);
    compare(moved, values);
    compare(lis, {});
    copy.clear();
    compare(copy, {});
}

struct MoveOnlyValue
{
    explicit MoveOnlyValue(int n) : number(n)
    {
    }
    MoveOnlyValue(const MoveOnlyValue &) = delete;
    MoveOnlyValue(MoveOnlyValue &&) noexcept = default;

    bool operator<(const MoveOnlyValue &other) const noexcept
    {
        return number < other.number;
    }

    int number;
};
} // namespace

int main()
{
    randomized_history<true>();
    randomized_history<false>();
    sirkit::RollbackLIS<int> large;
    large.reserve(200'000);
    for (int i = 0; i < 200'000; ++i)
    {
        large.push_back(i);
    }
    assert(large.length() == 200'000);
    large.rollback(100'000);
    assert(large.length() == 100'000);
    assert(large.indices().back() == 99'999);

    sirkit::RollbackLIS<MoveOnlyValue> move_only;
    move_only.push_back(MoveOnlyValue(1));
    move_only.push_back(MoveOnlyValue(2));
    assert(move_only.length() == 2);

    sirkit::RollbackLIS<int> lis;
    bool allocation_failed = false;
    remaining_allocations = 1;
    try
    {
        lis.push_back(5);
    }
    catch (const std::bad_alloc &)
    {
        allocation_failed = true;
    }
    remaining_allocations = kNoAllocationLimit;
    assert(allocation_failed);
    compare(lis, {});
    lis.push_back(7);
    compare(lis, {7});

#ifndef SKIP_RUNTIME_CHECKS
    bool invalid_rollback = false;
    try
    {
        lis.rollback(2);
    }
    catch (const std::out_of_range &)
    {
        invalid_rollback = true;
    }
    assert(invalid_rollback);
    compare(lis, {7});
    lis.pop_back();
    bool invalid_pop = false;
    try
    {
        lis.pop_back();
    }
    catch (const std::out_of_range &)
    {
        invalid_pop = true;
    }
    assert(invalid_pop);
#endif
}
