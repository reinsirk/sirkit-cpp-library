// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Meldable_Heap.cpp"

#include <array>
#include <cassert>
#include <functional>
#include <iterator>
#include <random>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
using Heap = sirkit::MeldableHeap<int>;

template<class Exception, class Operation>
void expect_exception(Operation operation)
{
    bool thrown = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}

template<class Queue>
void check_and_drain(Queue &heap, const std::multiset<int> &reference)
{
    assert(heap.size() == reference.size());
    for (auto at = reference.rbegin(); at != reference.rend(); ++at)
    {
        assert(heap.top() == *at);
        heap.pop();
    }
    assert(heap.empty());
}

void check_api()
{
    static_assert(!std::is_copy_constructible_v<Heap>);
    static_assert(!std::is_copy_assignable_v<Heap>);
    static_assert(std::is_nothrow_move_constructible_v<Heap>);
    static_assert(std::is_nothrow_move_assignable_v<Heap>);
    Heap a, b;
    assert(a.empty() && a.size() == 0);
    a.meld(b);
    assert(a.empty() && b.empty());
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::out_of_range>([&] { static_cast<void>(a.top()); });
    expect_exception<std::out_of_range>([&] { a.pop(); });
#endif
    const int first = 2;
    a.push(first);
    b.emplace(7);
    b.push(4);
    const int *address = &b.top();
    a.meld(b);
    assert(a.size() == 3 && a.top() == 7 && &a.top() == address && b.empty());
    a.meld(a);
    a.meld(b);
    assert(a.size() == 3 && a.top() == 7);
    Heap moved(std::move(a));
    assert(a.empty() && moved.size() == 3);
    a.push(90);
    auto &same = moved;
    moved = std::move(same);
    assert(moved.size() == 3 && moved.top() == 7);
    a = std::move(moved);
    assert(moved.empty() && a.size() == 3 && a.top() == 7);
    check_and_drain(a, {2, 4, 7});
    a.clear();

    sirkit::MeldableHeap<int, std::greater<int>> smaller, another(std::greater<int>{});
    smaller.push(4);
    another.push(1);
    another.push(4);
    smaller.meld(another);
    assert(another.empty() && smaller.top() == 1);
    smaller.pop();
    assert(smaller.top() == 4 && smaller.size() == 2);
}

struct ControlledCompare
{
    int &budget;
    int &calls;
    bool minimum;

    bool operator()(const int &a, const int &b) const
    {
        ++calls;
        if (budget == 0)
        {
            throw std::logic_error("comparison failure");
        }
        if (budget > 0)
        {
            --budget;
        }
        return minimum ? a > b : a < b;
    }
};

void check_exception_paths()
{
    using ControlledHeap = sirkit::MeldableHeap<int, ControlledCompare>;
    int budget = -1, calls = 0;
    const ControlledCompare max_order{budget, calls, false};
    const ControlledCompare min_order{budget, calls, true};
    ControlledHeap smaller(min_order), extra(min_order);
    smaller.push(7);
    extra.push(6);
    extra.push(9);
    smaller.meld(extra);
    assert(smaller.top() == 6 && extra.empty());
    static_assert(std::is_nothrow_move_constructible_v<ControlledHeap>);
    static_assert(!std::is_move_assignable_v<ControlledHeap>);
    auto minimum = std::move(smaller);
    minimum.push(1);
    assert(smaller.empty() && minimum.top() == 1);

    std::multiset<int> even, odd;
    for (int v = 255; v >= 0; --v)
    {
        (v % 2 ? odd : even).insert(v);
    }
    const auto make = [&](const std::multiset<int> &values) {
        ControlledHeap heap(max_order);
        for (auto it = values.rbegin(); it != values.rend(); ++it)
        {
            heap.push(*it);
        }
        return heap;
    };
    int meld_comparisons{};
    {
        auto a = make(even);
        auto b = make(odd);
        calls = 0;
        a.meld(b);
        meld_comparisons = calls;
    }
    assert(meld_comparisons >= 3);
    for (int stop = 0; stop < meld_comparisons; ++stop)
    {
        auto a = make(even);
        auto b = make(odd);
        budget = stop;
        expect_exception<std::logic_error>([&] { a.meld(b); });
        budget = -1;
        check_and_drain(a, even);
        check_and_drain(b, odd);
    }
    int pop_comparisons{};
    {
        auto a = make(odd);
        calls = 0;
        a.pop();
        pop_comparisons = calls;
    }
    assert(pop_comparisons >= 3);
    for (int stop = 0; stop < pop_comparisons; ++stop)
    {
        auto a = make(odd);
        budget = stop;
        expect_exception<std::logic_error>([&] { a.pop(); });
        budget = -1;
        check_and_drain(a, odd);
    }
    auto a = make(even);
    budget = 0;
    expect_exception<std::logic_error>([&] { a.emplace(32); });
    budget = -1;
    check_and_drain(a, even);
}

struct Entry
{
    int value;
    int *active;

    Entry(int v, int &live) : value(v), active(&live)
    {
        if (v < 0)
        {
            throw std::invalid_argument("entry construction failure");
        }
        ++*active;
    }

    Entry(const Entry &) = delete;
    Entry(Entry &&) = delete;

    ~Entry()
    {
        --*active;
    }
};

struct ByEntry
{
    bool operator()(const Entry &a, const Entry &b) const
    {
        return a.value < b.value;
    }
};

void check_element_lifetime()
{
    int live = 0;
    {
        sirkit::MeldableHeap<Entry, ByEntry> a, b;
        a.emplace(4, live);
        b.emplace(9, live);
        b.emplace(6, live);
        expect_exception<std::invalid_argument>([&] { a.emplace(-1, live); });
        assert(live == 3 && a.size() == 1 && a.top().value == 4);
        a.meld(b);
        assert(live == 3 && a.size() == 3 && a.top().value == 9 && b.empty());
        a.pop();
        assert(live == 2 && a.top().value == 6);
        b.emplace(20, live);
        a = std::move(b);
        assert(live == 1 && a.top().value == 20 && b.empty());
    }
    assert(live == 0);
}

void test_random()
{
    std::mt19937 rng(348175);
    std::array<Heap, 17> heaps;
    std::array<std::multiset<int>, 17> expected;
    for (int step = 0; step < 100000; ++step)
    {
        const auto i = rng() % heaps.size(), j = rng() % heaps.size();
        const int operation = rng() % 12;
        if (operation < 7)
        {
            const int x = static_cast<int>(rng() % 201) - 100;
            heaps[i].push(x);
            expected[i].insert(x);
        }
        else if (operation == 7 && !expected[i].empty())
        {
            heaps[i].pop();
            expected[i].erase(std::prev(expected[i].end()));
        }
        else if (operation < 10)
        {
            heaps[i].meld(heaps[j]);
            if (i != j)
            {
                expected[i].merge(expected[j]);
            }
        }
        else if (operation == 10)
        {
            heaps[i] = std::move(heaps[j]);
            if (i != j)
            {
                expected[i] = std::move(expected[j]);
                expected[j].clear();
            }
        }
        else
        {
            heaps[i].clear();
            expected[i].clear();
        }
        for (const auto k : {i, j})
        {
            assert(heaps[k].size() == expected[k].size());
            assert(heaps[k].empty() == expected[k].empty());
            if (!expected[k].empty())
            {
                assert(heaps[k].top() == *expected[k].rbegin());
            }
        }
    }
    for (std::size_t i = 0; i < heaps.size(); ++i)
    {
        check_and_drain(heaps[i], expected[i]);
    }
}

void test_tall_destruction()
{
    int live = 0;
    {
        sirkit::MeldableHeap<Entry, ByEntry> tall, smaller;
        for (int v = 0; v < 350000; ++v)
        {
            // 毎回より大きな根ができ、左の子だけが続く。
            tall.emplace(v, live);
        }
        assert(tall.top().value == 349999 && tall.size() == 350000 && live == 350000);
        smaller.emplace(42, live);
        tall = std::move(smaller);
        assert(tall.top().value == 42 && smaller.empty() && live == 1);
        for (int v = 0; v < 350000; ++v)
        {
            smaller.emplace(v, live);
        }
        assert(smaller.top().value == 349999 && smaller.size() == 350000 && live == 350001);
    }
    assert(live == 0);
}
}

int main()
{
    check_api();
    check_exception_paths();
    check_element_lifetime();
    test_random();
    test_tall_destruction();
}
