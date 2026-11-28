// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Range_Equal_Runs.cpp"

#include <array>
#include <cassert>
#include <random>

struct CheckedValue
{
    int number;
    static inline int comparisons_left = -1;

    friend bool operator==(const CheckedValue &first, const CheckedValue &second)
    {
        if (comparisons_left == 0)
        {
            throw std::runtime_error("comparison failed");
        }
        if (comparisons_left > 0)
        {
            --comparisons_left;
        }
        return first.number == second.number;
    }

    friend bool operator==(CheckedValue &, const CheckedValue &) = delete;
};

static sirkit::EqualRunLengths brute(const std::vector<int> &values, std::size_t left, std::size_t right)
{
    if (left == right)
    {
        return {};
    }
    std::size_t prefix = left + 1, suffix = right - 1;
    while (prefix < right && values[prefix] == values[left])
    {
        ++prefix;
    }
    while (suffix > left && values[suffix - 1] == values[right - 1])
    {
        --suffix;
    }
    std::uint32_t longest = 1, current = 1;
    for (std::size_t i = left + 1; i < right; ++i)
    {
        current = values[i - 1] == values[i] ? current + 1 : 1;
        longest = std::max(longest, current);
    }
    return {static_cast<std::uint32_t>(right - left), static_cast<std::uint32_t>(prefix - left),
            static_cast<std::uint32_t>(right - suffix), longest};
}

static void expect(sirkit::EqualRunLengths first, sirkit::EqualRunLengths second)
{
    assert(first.length == second.length && first.prefix == second.prefix &&
           first.suffix == second.suffix && first.longest == second.longest);
}

int main()
{
    const sirkit::RangeEqualRuns<int> empty;
    assert(empty.empty() && empty.size() == 0);
    expect(empty.all(), {});
    expect(empty.query(0, 0), {});
    const sirkit::RangeEqualRuns<int> empty_span(std::span<const int>{});
    expect(empty_span.all(), {});

    const std::array one{2};
    sirkit::RangeEqualRuns<int> singleton(one);
    expect(singleton.all(), {1, 1, 1, 1});
    singleton.set(0, 5);
    expect(singleton.query(0, 1), {1, 1, 1, 1});
    assert(singleton.get(0) == 5);

    const std::array initial{1, 1, 2, 2, 2, 3};
    sirkit::RangeEqualRuns<int> runs(initial);
    expect(runs.all(), {6, 2, 1, 3});
    runs.set(5, 2);
    runs.set(0, 2);
    expect(runs.all(), {6, 1, 4, 4});
    assert(runs.get(0) == 2);
    runs.set(1, 2);
    expect(runs.all(), {6, 6, 6, 6});

    auto copy = runs;
    copy.set(3, 1);
    assert(copy.all().longest == 3 && runs.all().longest == 6);
    copy = runs;
    auto moved = std::move(copy);
    assert(copy.empty() && moved.all().longest == 6);
    copy = std::move(moved);
    assert(moved.empty());
    auto &alias = copy;
    copy = std::move(alias);
    assert(copy.all().longest == 6);
    const std::array bool_row{false, false, true, false};
    sirkit::RangeEqualRuns<bool> booleans(bool_row);
    booleans.set(2, false);
    assert(!booleans.get(2) && booleans.all().longest == 4);

    const std::array checked{CheckedValue{1}, CheckedValue{2}, CheckedValue{1}};
    sirkit::RangeEqualRuns<CheckedValue> transactional(checked);
    CheckedValue::comparisons_left = 1;
    bool threw = false;
    try
    {
        transactional.set(1, CheckedValue{1});
    }
    catch (const std::runtime_error &)
    {
        threw = true;
    }
    CheckedValue::comparisons_left = -1;
    assert(threw && transactional.get(1).number == 2 && transactional.all().longest == 1);
    transactional.set(1, CheckedValue{1});
    assert(transactional.all().longest == 3);

    bool length_failed = false;
    try
    {
        sirkit::detail::ValidateEqualRunLength(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
    }
    catch (const std::length_error &)
    {
        length_failed = true;
    }
    assert(length_failed);
#ifndef SKIP_RUNTIME_CHECKS
    bool point_failed = false;
    try
    {
        runs.set(runs.size(), 1);
    }
    catch (const std::out_of_range &)
    {
        point_failed = true;
    }
    assert(point_failed);
    for (const auto &[left, right] : {std::pair{5U, 4U}, std::pair{1U, 7U}})
    {
        bool interval_failed = false;
        try
        {
            (void)runs.query(left, right);
        }
        catch (const std::out_of_range &)
        {
            interval_failed = true;
        }
        assert(interval_failed);
    }
#endif

    std::mt19937 rng(3262);
    for (int trial = 0; trial < 300; ++trial)
    {
        const auto n = 1 + rng() % 87;
        std::vector<int> values(n);
        for (auto &v : values)
        {
            v = static_cast<int>(rng() % 6);
        }
        sirkit::RangeEqualRuns<int> fast(values);
        for (int query = 0; query < 120; ++query)
        {
            const int next = static_cast<int>(rng() % 6);
            const auto position = rng() % n;
            fast.set(position, next);
            values[position] = next;
            const auto a = rng() % (n + 1);
            const auto b = rng() % (n + 1);
            const auto left = std::min(a, b);
            const auto right = std::max(a, b);
            expect(fast.query(left, right), brute(values, left, right));
            expect(fast.all(), brute(values, 0, n));
        }
    }
}
