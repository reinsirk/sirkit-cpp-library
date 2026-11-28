// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Ordered_Multiset.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>

using sirkit::OrderedMultiset;
using Count = std::uint64_t;

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
    OrderedMultiset<int> values;
    assert(values.empty() && values.count(0) == 0 && !values.kth(0));
    values.insert(0, 0);
    assert(values.erase(0) == 0 && values.erase(0, 0) == 0 && values.empty());

    constexpr Count maximum = std::numeric_limits<Count>::max();
    values.insert(0, maximum);
    assert(values.size() == maximum && values.count(0) == maximum && *values.kth(maximum - 1) == 0);
    assert(!values.kth(maximum));
    expect_error<std::length_error>([&] { values.insert(1); });
    assert(values.count_less_equal(0) == maximum && values.count_less(0) == 0);
    assert(values.erase(0, maximum - 4) == maximum - 4);
    values.insert(-1, 3);
    values.insert(1, 2);
    assert(values.size() == 9 && values.count_less_equal(0) == 7 && *values.kth(8) == 1);
    assert(values.erase(0, 0) == 0 && values.count(0) == 4);
    assert(values.erase(0, 999) == 4);
    assert(values.erase(-1, 999) == 3);
    assert(values.erase(1, 999) == 2 && values.empty());
}

void rotations_and_ownership()
{
    for (const auto &order : {std::array{3, 2, 1}, std::array{1, 2, 3}, std::array{3, 1, 2}, std::array{1, 3, 2}})
    {
        OrderedMultiset<int> values;
        for (const int value : order)
        {
            values.insert(value);
        }
        assert(*values.kth(0) == 1 && *values.kth(1) == 2 && *values.kth(2) == 3);
    }

    OrderedMultiset<int> original;
    for (const auto value : {4, 2, 6, 1, 3, 5, 7, 8})
    {
        original.insert(value);
    }
    const int *saved = original.kth(4);
    assert(*saved == 5);
    // Erasing a borrowed key must not use that reference after destroying its node.
    assert(original.erase(*original.kth(3)) == 1);
    assert(original.size() == 7 && original.kth(3) == saved);
    OrderedMultiset<int> moved(std::move(original));
    const auto self = &moved;
    moved = std::move(*self);
    assert(original.empty() && moved.kth(3) == saved);
    original.insert(99);
    original = std::move(moved);
    assert(moved.empty() && original.kth(3) == saved && !original.count(99));
}

struct ThrowingCompare
{
    int *before_throw;

    bool operator()(int a, int b) const
    {
        if (*before_throw == 0)
        {
            throw std::runtime_error("comparison failed");
        }
        if (*before_throw > 0)
        {
            --*before_throw;
        }
        return a < b;
    }
};

struct MoveOnlyKey
{
    inline static bool reject_move = false;
    int value;

    explicit MoveOnlyKey(int initial) : value(initial)
    {
    }

    MoveOnlyKey(const MoveOnlyKey &) = delete;
    MoveOnlyKey(MoveOnlyKey &&other) : value(other.value)
    {
        if (reject_move)
        {
            throw std::runtime_error("key move failed");
        }
    }

    bool operator<(const MoveOnlyKey &other) const
    {
        return value < other.value;
    }
};

void exception_cases()
{
    int before_throw = -1;
    OrderedMultiset<int, ThrowingCompare> values(ThrowingCompare{&before_throw});
    for (const int key : {4, 2, 6, 3})
    {
        values.insert(key);
    }
    before_throw = 1;
    expect_error<std::runtime_error>([&] { values.insert(1); });
    before_throw = 1;
    expect_error<std::runtime_error>([&] { static_cast<void>(values.erase(2)); });
    before_throw = -1;
    assert(values.size() == 4 && values.count(2) == 1 && values.count(1) == 0);

    OrderedMultiset<MoveOnlyKey> movable;
    for (const int key : {4, 2, 6, 1, 3, 5, 7, 8})
    {
        movable.insert(MoveOnlyKey(key));
    }
    MoveOnlyKey::reject_move = true;
    expect_error<std::runtime_error>([&] { movable.insert(MoveOnlyKey(9)); });
    assert(movable.size() == 8 && movable.count(MoveOnlyKey(9)) == 0);
    const auto *five = movable.kth(4);
    assert(movable.erase(MoveOnlyKey(4)) == 1 && movable.kth(3) == five);
    MoveOnlyKey::reject_move = false;
}

template <typename Compare>
void random_cases(Compare compare, unsigned seed)
{
    std::mt19937 random(seed);
    for (int test = 0; test < 110; ++test)
    {
        OrderedMultiset<int, Compare> values(compare);
        std::multiset<int, Compare> expected(compare);
        for (int step = 0; step < 440; ++step)
        {
            const int value = static_cast<int>(random() % 101) - 50;
            const Count copies = random() % 9;
            if (random() % 2 == 0)
            {
                values.insert(value, copies);
                for (Count i = 0; i < copies; ++i)
                {
                    expected.insert(value);
                }
            }
            else
            {
                auto found = expected.lower_bound(value);
                const auto end = expected.upper_bound(value);
                Count erased = 0;
                while (found != end && erased < copies)
                {
                    found = expected.erase(found);
                    ++erased;
                }
                assert(values.erase(value, copies) == erased);
            }
            assert(values.size() == expected.size() && values.empty() == expected.empty());

            const int probe = static_cast<int>(random() % 121) - 60;
            const auto lower = expected.lower_bound(probe), upper = expected.upper_bound(probe);
            assert(values.count(probe) == static_cast<Count>(std::distance(lower, upper)));
            assert(values.count_less(probe) == static_cast<Count>(std::distance(expected.begin(), lower)));
            assert(values.count_less_equal(probe) == static_cast<Count>(std::distance(expected.begin(), upper)));
            const auto rank = random() % (expected.size() + 1);
            if (rank == expected.size())
            {
                assert(!values.kth(rank));
            }
            else
            {
                const auto selected = std::next(expected.begin(), static_cast<std::ptrdiff_t>(rank));
                assert(values.kth(rank) && *values.kth(rank) == *selected);
            }
        }
    }
}

struct CountComparisons
{
    int *calls;

    bool operator()(int a, int b) const
    {
        ++*calls;
        return a < b;
    }
};

void sorted_stress()
{
    int calls = 0;
    constexpr int n = 25000;
    OrderedMultiset<int, CountComparisons> values(CountComparisons{&calls});
    for (int i = 0; i < n; ++i)
    {
        values.insert(i);
    }
    for (int i = 1; i < n; i += 2)
    {
        assert(values.erase(i) == 1);
    }
    assert(values.size() == n / 2 && calls < 100 * n);
    for (int i = 0; i < n / 2; ++i)
    {
        assert(*values.kth(static_cast<Count>(i)) == 2 * i);
    }
}

int main()
{
    edge_cases();
    rotations_and_ownership();
    exception_cases();
    random_cases(std::less<>{}, 7182);
    random_cases(std::greater<>{}, 18007);
    sorted_stress();
}
