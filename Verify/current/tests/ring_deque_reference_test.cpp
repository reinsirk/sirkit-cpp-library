// SPDX-License-Identifier: CC0-1.0
#include "DataStructure/Ring_Deque.cpp"

#include <cstdlib>
#include <deque>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <utility>

namespace
{
void Expect(bool predicate)
{
    if (!predicate)
    {
        std::abort();
    }
}

template <class Error, class F>
void ExpectThrow(F method)
{
    try
    {
        method();
    }
    catch (const Error &)
    {
        return;
    }
    std::abort();
}

void Check(const sirkit::RingDeque<int> &data, const std::deque<int> &reference)
{
    Expect(data.size() == reference.size());
    Expect(data.empty() == reference.empty());
    Expect(data.capacity() >= data.size());
    for (std::size_t i = 0; i < reference.size(); ++i)
    {
        Expect(data[i] == reference[i] && data.at(i) == reference[i]);
    }
    if (!reference.empty())
    {
        Expect(data.front() == reference.front() && data.back() == reference.back());
    }
}

void Random()
{
    std::mt19937 rng(5138304);
    for (int trial = 0; trial < 120; ++trial)
    {
        sirkit::RingDeque<int> data;
        std::deque<int> reference;
        for (int i = 0; i < 600; ++i)
        {
            const int base = rng() % 2000;
            const int kind = reference.empty() ? rng() % 2 : rng() % 10;
            switch (kind)
            {
            case 0:
                data.push_back(base);
                reference.push_back(base);
                break;
            case 1:
                data.push_front(base);
                reference.push_front(base);
                break;
            case 2:
                data.pop_front();
                reference.pop_front();
                break;
            case 3:
                data.pop_back();
                reference.pop_back();
                break;
            case 4:
                data.front() = reference.front() = base;
                break;
            case 5:
                data.back() = reference.back() = base;
                break;
            case 6:
            {
                const auto index = rng() % reference.size();
                data[index] = reference[index] = base;
                break;
            }
            case 7:
                data.reserve(rng() % 420);
                break;
            case 8:
                data.clear();
                reference.clear();
                break;
            case 9:
                data.at(0) = reference.front() = base;
                break;
            }
            Check(data, reference);
        }
    }
}

struct ThrowingCopy
{
    inline static int remaining = -1;
    int value;

    explicit ThrowingCopy(int v) : value(v)
    {
    }

    ThrowingCopy(const ThrowingCopy &other) : value(other.value)
    {
        if (remaining == 0)
        {
            throw std::runtime_error("element copy");
        }
        if (remaining > 0)
        {
            --remaining;
        }
    }

    ThrowingCopy(ThrowingCopy &&other) noexcept(false) : value(other.value)
    {
        other.value = -999;
    }
};

void Boundaries()
{
    sirkit::RingDeque<int> data;
    Expect(data.capacity() == 0);
    Expect(data.emplace_back() == 0);
    data.clear();
    const auto capacity = data.capacity();
    for (std::size_t i = 0; i < capacity; ++i)
    {
        data.push_back(static_cast<int>(i + 1));
    }
    data.push_front(data.back()); // aliasing during growth
    Expect(data.front() == static_cast<int>(capacity) && data.size() == capacity + 1);
    for (std::size_t i = 1; i <= capacity; ++i)
    {
        Expect(data[i] == static_cast<int>(i));
    }
    const auto copy = data;
    data.clear();
    Expect(copy.size() == capacity + 1);
    data = copy;
    auto moved = std::move(data);
    Expect(data.empty() && moved.front() == copy.front());
    data = std::move(moved);
    Expect(moved.empty() && data.at(1) == 1);

    sirkit::RingDeque<std::unique_ptr<int>> only;
    only.push_back(std::make_unique<int>(8));
    only.push_front(std::make_unique<int>(-5));
    only.reserve(64);
    Expect(*only.front() == -5 && *only.back() == 8);
    only.pop_back();
    Expect(*only.front() == -5 && only.size() == 1);

    sirkit::RingDeque<ThrowingCopy> fragile;
    fragile.emplace_back(1);
    fragile.emplace_front(2);
    fragile.emplace_back(3);
    const auto original_capacity = fragile.capacity();
    ThrowingCopy::remaining = 1;
    ExpectThrow<std::runtime_error>([&]
                                    {
                                        fragile.reserve(original_capacity + 1);
                                    });
    Expect(fragile.capacity() == original_capacity);
    Expect(fragile.at(0).value == 2 && fragile.at(1).value == 1 && fragile.at(2).value == 3);
    ThrowingCopy::remaining = -1;
    fragile.reserve(original_capacity + 1);
    Expect(fragile.at(0).value == 2 && fragile.at(1).value == 1 && fragile.at(2).value == 3);

    ExpectThrow<std::length_error>([&]
                                   {
                                       data.reserve(std::numeric_limits<std::size_t>::max());
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)data.at(data.size());
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)copy.at(copy.size());
                                   });
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)moved.front();
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       moved.pop_back();
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       moved.pop_front();
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)copy[copy.size()];
                                   });
#endif
}
}

int main()
{
    Random();
    Boundaries();
}
