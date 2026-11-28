// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Interval_Map.cpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <random>

struct ThrowingValue
{
    int value;
    static inline int copies_left = -1;
    static inline int comparisons_left = -1;

    explicit ThrowingValue(int v) : value(v) {}
    ThrowingValue(const ThrowingValue &other) : value(other.value)
    {
        consume(copies_left);
    }
    ThrowingValue &operator=(const ThrowingValue &) = delete;

    friend bool operator==(const ThrowingValue &a, const ThrowingValue &b)
    {
        consume(comparisons_left);
        return a.value == b.value;
    }

private:
    static void consume(int &budget)
    {
        if (budget == 0)
        {
            throw std::runtime_error("test budget exhausted");
        }
        if (budget > 0)
        {
            --budget;
        }
    }
};

static void check_shape(const sirkit::IntervalMap<int> &fast, const std::vector<int> &slow)
{
    assert(fast.size() == slow.size() && fast.empty() == slow.empty());
    const auto runs = fast.runs();
    assert(fast.run_count() == runs.size());
    std::size_t previous = 0;
    for (const auto &[left, right, value] : runs)
    {
        assert(left == previous && left < right && right <= slow.size());
        assert(left == 0 || slow[left - 1] != value);
        assert(right == slow.size() || slow[right] != value);
        for (std::size_t i = left; i < right; ++i)
        {
            assert(slow[i] == value && fast.get(i) == value);
            const auto run = fast.run_at(i);
            assert(run.left == left && run.right == right && run.value == value);
        }
        previous = right;
    }
    assert(previous == slow.size());
}

int main()
{
    sirkit::IntervalMap<int> empty;
    check_shape(empty, {});
    empty.assign(0, 0, 1);
    check_shape(sirkit::IntervalMap<int>(0, 5), {});
    check_shape(sirkit::IntervalMap<int>(std::span<const int>{}), {});

    sirkit::IntervalMap<int> values(8, 0);
    values.assign(1, 4, 7);
    values.assign(4, 6, 7);
    check_shape(values, {0, 7, 7, 7, 7, 7, 0, 0});
    values.assign(1, 3, 7);
    values.assign(7, 8, 0);
    check_shape(values, {0, 7, 7, 7, 7, 7, 0, 0});
    values.assign(0, 8, values.get(4));
    check_shape(values, std::vector<int>(8, 7));
    values.assign(7, 8, 8);
    values.assign(5, 7, 8);
    values.assign(6, 8, 9);
    check_shape(values, {7, 7, 7, 7, 7, 8, 9, 9});

    auto copy = values;
    copy.assign(0, 8, 2);
    assert(values.get(0) == 7 && copy.get(0) == 2);
    copy = values;
    check_shape(copy, {7, 7, 7, 7, 7, 8, 9, 9});
    auto moved = std::move(copy);
    check_shape(copy, {});
    copy = std::move(moved);
    auto &alias = copy;
    copy = std::move(alias);
    check_shape(copy, {7, 7, 7, 7, 7, 8, 9, 9});
    assert(moved.empty());

    constexpr auto kSize = std::numeric_limits<std::size_t>::max();
    sirkit::IntervalMap<int> huge(kSize, 1);
    huge.assign(kSize - 3, kSize, 9);
    assert(huge.run_at(kSize - 1).left == kSize - 3);
    assert(huge.run_at(kSize - 1).right == kSize);
    assert(huge.get(kSize - 4) == 1 && huge.get(kSize - 1) == 9);

#ifndef SKIP_RUNTIME_CHECKS
    bool bad_point = false;
    try
    {
        (void)values.get(values.size());
    }
    catch (const std::out_of_range &)
    {
        bad_point = true;
    }
    assert(bad_point);
    for (const auto &[left, right] : {std::pair{7U, 6U}, std::pair{2U, 9U}})
    {
        bool bad_interval = false;
        try
        {
            values.assign(left, right, 10);
        }
        catch (const std::out_of_range &)
        {
            bad_interval = true;
        }
        assert(bad_interval);
    }
#endif

    sirkit::IntervalMap<ThrowingValue> transactional(8, ThrowingValue(11));
    bool copy_threw = false;
    ThrowingValue::copies_left = 1;
    try
    {
        transactional.assign(2, 6, ThrowingValue(6));
    }
    catch (const std::runtime_error &)
    {
        copy_threw = true;
    }
    ThrowingValue::copies_left = -1;
    assert(copy_threw && transactional.run_count() == 1 && transactional.get(5).value == 11);

    bool comparison_threw = false;
    ThrowingValue::comparisons_left = 1;
    try
    {
        transactional.assign(2, 6, ThrowingValue(6));
    }
    catch (const std::runtime_error &)
    {
        comparison_threw = true;
    }
    ThrowingValue::comparisons_left = -1;
    assert(comparison_threw && transactional.run_count() == 1 && transactional.get(5).value == 11);
    transactional.assign(2, 6, ThrowingValue(6));
    assert(transactional.run_count() == 3 && transactional.get(5).value == 6);

    std::mt19937 rng(53831);
    for (int test = 0; test < 150; ++test)
    {
        std::vector<int> model(1 + rng() % 81);
        for (auto &value : model)
        {
            value = static_cast<int>(rng() % 6);
        }
        sirkit::IntervalMap<int> fast(model);
        check_shape(fast, model);
        for (int query = 0; query < 150; ++query)
        {
            const auto a = rng() % (model.size() + 1);
            const auto b = rng() % (model.size() + 1);
            const auto left = std::min(a, b);
            const auto right = std::max(a, b);
            const int value = static_cast<int>(rng() % 6);
            fast.assign(left, right, value);
            std::fill(model.begin() + left, model.begin() + right, value);
            check_shape(fast, model);
        }
    }
}
