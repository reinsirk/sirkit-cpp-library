// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Deque_Aggregation.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <random>
#include <stdexcept>
#include <utility>

namespace
{
constexpr std::uint64_t kMod = 998244353;

struct Affine
{
    std::uint64_t a, b;

    bool operator==(const Affine &) const noexcept = default;
};

struct Compose
{
    Affine operator()(const Affine &left, const Affine &right) const noexcept
    {
        return {right.a * left.a % kMod, (right.a * left.b + right.b) % kMod};
    }
};

template <class Exception, class F>
void ExpectThrow(F action)
{
    bool thrown = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}

using Deque = sirkit::DequeAggregation<Affine, Compose>;
constexpr Affine kIdentity{1, 0};

void Check(const Deque &test, const std::deque<Affine> &reference)
{
    assert(test.empty() == reference.empty());
    assert(test.size() == reference.size());
    if (!reference.empty())
    {
        assert(test.front() == reference.front());
        assert(test.back() == reference.back());
    }
    std::uint64_t zero = 0, one = 1;
    for (const auto &f : reference)
    {
        zero = (f.a * zero + f.b) % kMod;
        one = (f.a * one + f.b) % kMod;
    }
    assert((test.all_prod() == Affine{(one + kMod - zero) % kMod, zero}));
}

void CheckEdges()
{
    Deque test(kIdentity);
    Check(test, {});
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)test.front();
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)test.back();
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       test.pop_front();
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       test.pop_back();
                                   });
#endif
    test.push_back({3, 5});
    Check(test, {{3, 5}});
    test.pop_front();
    Check(test, {});
    test.push_front({7, 11});
    Check(test, {{7, 11}});
    test.pop_back();
    Check(test, {});

    test.push_front({2, 0});
    test.push_back({1, 3});
    test.push_front({1, 2});
    test.push_back(test.front());
    const std::deque<Affine> reference{{1, 2}, {2, 0}, {1, 3}, {1, 2}};
    Check(test, reference);
    Deque copy = test;
    Deque moved = std::move(copy);
    Check(copy, {});
    Check(moved, reference);
    copy = moved;
    moved.clear();
    Check(copy, reference);
    Check(moved, {});
    moved = std::move(test);
    Check(moved, reference);
    Check(test, {});
    test.swap(moved);
    test.swap(test);
    Check(test, reference);
    Check(moved, {});
}

void CheckRandom()
{
    std::mt19937_64 random(0x94d30d93159d3c51ULL);
    Deque test(kIdentity);
    std::deque<Affine> reference;
    for (auto i = 0; i < 60'000; ++i)
    {
        auto operation = random() % 6;
        if (reference.empty())
        {
            operation %= 2;
        }
        if (operation == 0)
        {
            Affine f{random() % kMod, random() % kMod};
            test.push_front(f);
            reference.push_front(f);
        }
        else if (operation == 1)
        {
            Affine f{random() % kMod, random() % kMod};
            test.push_back(f);
            reference.push_back(f);
        }
        else if (operation == 2)
        {
            test.pop_front();
            reference.pop_front();
        }
        else if (operation == 3)
        {
            test.pop_back();
            reference.pop_back();
        }
        Check(test, reference);
    }
}

void CheckOneSide()
{
    constexpr std::size_t n = 10'003;
    Deque test(kIdentity);
    std::deque<Affine> reference;
    for (auto direction : {false, true})
    {
        for (std::size_t i = 0; i < n; ++i)
        {
            const Affine value{1 + i % 7, i};
            if (direction)
            {
                test.push_front(value);
                reference.push_front(value);
            }
            else
            {
                test.push_back(value);
                reference.push_back(value);
            }
        }
        for (std::size_t i = 0; i < n; ++i)
        {
            if (i % 991 == 0)
            {
                Check(test, reference);
            }
            if (direction)
            {
                test.pop_back();
                reference.pop_back();
            }
            else
            {
                test.pop_front();
                reference.pop_front();
            }
        }
        Check(test, {});
    }
}

struct BudgetedAdd
{
    int *budget;
    int bias;

    int operator()(int a, int b) const
    {
        if (*budget == 0)
        {
            throw std::runtime_error("budget");
        }
        if (*budget > 0)
        {
            --*budget;
        }
        return a + b + bias;
    }
};

void CheckOperationFailure()
{
    int budget = -1;
    sirkit::DequeAggregation test(0, BudgetedAdd{&budget, 0});
    for (auto i = 1; i <= 8; ++i)
    {
        test.push_back(i);
    }
    budget = 0;
    ExpectThrow<std::runtime_error>([&]
                                    {
                                        test.push_back(9);
                                    });
    ExpectThrow<std::runtime_error>([&]
                                    {
                                        test.pop_front();
                                    });
    budget = 3;
    ExpectThrow<std::runtime_error>([&]
                                    {
                                        test.pop_front();
                                    }); // 残る側の再計算中
    budget = -1;
    assert(test.size() == 8 && test.front() == 1 && test.back() == 8 && test.all_prod() == 36);
    test.pop_front();
    assert(test.size() == 7 && test.front() == 2 && test.all_prod() == 35);

    sirkit::DequeAggregation other(-3, BudgetedAdd{&budget, 3});
    for (auto i = 1; i <= 8; ++i)
    {
        other.push_front(i);
    }
    other.pop_back();
    assert(other.size() == 7 && other.back() == 2 && other.all_prod() == 53);
    test.swap(other);
    assert(test.all_prod() == 53 && other.all_prod() == 35);
    assert(other.front() == 2 && test.front() == 8);
}

struct BudgetedValue
{
    int value;
    int *budget;

    BudgetedValue(int v, int *amount) noexcept : value(v), budget(amount)
    {
    }
    BudgetedValue(const BudgetedValue &other) : value(other.value), budget(other.budget)
    {
        if (*budget == 0)
        {
            throw std::runtime_error("copy budget");
        }
        if (*budget > 0)
        {
            --*budget;
        }
    }
    BudgetedValue(BudgetedValue &&) noexcept = default;
    BudgetedValue &operator=(const BudgetedValue &) = default;
    BudgetedValue &operator=(BudgetedValue &&) noexcept = default;
};

struct AddValue
{
    BudgetedValue operator()(const BudgetedValue &a, const BudgetedValue &b) const noexcept
    {
        return {a.value + b.value, a.budget};
    }
};

void CheckCopyFailure()
{
    int budget = -1;
    const BudgetedValue zero(0, &budget);
    sirkit::DequeAggregation test(zero, AddValue{});
    for (int i = 1; i <= 9; ++i)
    {
        test.push_front(BudgetedValue(i, &budget));
    }
    budget = 2;
    ExpectThrow<std::runtime_error>([&]
                                    {
                                        test.pop_back();
                                    });
    assert(test.size() == 9 && test.front().value == 9 && test.back().value == 1);
    budget = -1;
    assert(test.all_prod().value == 45);
    test.pop_back();
    assert(test.all_prod().value == 44);
}
}

int main()
{
    CheckEdges();
    CheckRandom();
    CheckOneSide();
    CheckOperationFailure();
    CheckCopyFailure();
}
