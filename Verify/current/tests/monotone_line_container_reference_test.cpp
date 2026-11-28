// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Monotone_Line_Container.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Int = std::int64_t;
constexpr auto kMin = std::numeric_limits<Int>::min();
constexpr auto kMax = std::numeric_limits<Int>::max();

template <class Exception, class Callback>
void must_throw(Callback call)
{
    bool caught = false;
    try
    {
        call();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_limits()
{
    sirkit::MonotoneMinLineContainer hull;
    hull.reserve(8);
    assert(hull.empty() && !hull.query(0) && !hull.query_monotone(kMin));
    hull.add(1, kMin);
    hull.add(0, kMax);
    assert(hull.size() == 1 && hull.query_monotone(kMax)->value == -1);
#ifndef SKIP_RUNTIME_CHECKS
    must_throw<std::invalid_argument>([&]
                                      {
                                          hull.add(1, 0);
                                      });
    must_throw<std::invalid_argument>([&]
                                      {
                                          static_cast<void>(hull.query_monotone(kMax - 1));
                                      });
#endif
    hull.clear();
    hull.add(1, kMax);
    hull.add(0, kMin);
    hull.add(0, 0);
    assert(hull.size() == 1 && hull.query(kMin)->value == kMin);
    sirkit::MonotoneMinLineContainer copy(hull);
    auto moved = std::move(copy);
    assert(copy.empty() && moved.query_monotone(3)->value == kMin);
    copy = moved;
    moved.add(-1, kMin);
    assert(copy.query(1)->value == kMin);
    must_throw<std::overflow_error>([&]
                                    {
                                        static_cast<void>(moved.query(1));
                                    });
    copy = std::move(moved);
    assert(moved.empty() && copy.query(-1)->value == kMin);

    sirkit::MonotoneMaxLineContainer max_hull;
    max_hull.add(kMin, 0);
    max_hull.add(kMax, 0);
    assert(max_hull.query(1)->value == kMax);
    must_throw<std::overflow_error>([&]
                                    {
                                        static_cast<void>(max_hull.query(-1));
                                    });
    max_hull.clear();
    max_hull.add(kMax, kMin);
    assert(max_hull.query(2)->value == kMax - 1);
    assert(max_hull.query(0)->value == kMin);
    must_throw<std::overflow_error>([&]
                                    {
                                        static_cast<void>(max_hull.query(3));
                                    });

    max_hull.clear();
    max_hull.add(kMax, kMax);
    assert(max_hull.query(-2)->value == -kMax);
    assert(max_hull.query(0)->value == kMax);
    assert(max_hull.query(-1)->value == 0);
    must_throw<std::overflow_error>([&]
                                    {
                                        static_cast<void>(max_hull.query(2));
                                    });
    must_throw<std::overflow_error>([&]
                                    {
                                        static_cast<void>(max_hull.query(1));
                                    });
}

template <bool Minimize>
void compare_random()
{
    std::mt19937 random(11571);
    for (int round = 0; round < 240; ++round)
    {
        sirkit::MonotoneLineContainer<Minimize> hull;
        std::vector<std::pair<Int, Int>> lines;
        Int slope = 0;
        Int previous_x = -250;
        for (int i = 0; i < 90; ++i)
        {
            slope += (Minimize ? -1 : 1) * static_cast<Int>(random() % 5);
            const Int intercept = static_cast<Int>(random() % 601) - 300;
            hull.add(slope, intercept);
            lines.emplace_back(slope, intercept);
            previous_x += static_cast<Int>(random() % 7);
            const Int other_x = static_cast<Int>(random() % 1001) - 500;
            const auto check = [&](auto result, Int x)
            {
                Int expected = lines.front().first * x + lines.front().second;
                for (const auto &[a, b] : lines)
                {
                    const Int value = a * x + b;
                    expected = Minimize ? std::min(expected, value) : std::max(expected, value);
                }
                assert(result && result->value == expected);
                assert(result->slope * x + result->intercept == expected);
                assert(std::ranges::find(lines, std::pair{result->slope, result->intercept}) != lines.end());
            };
            check(hull.query(other_x), other_x);
            check(hull.query_monotone(previous_x), previous_x);
        }
    }
}
}

int main()
{
    check_limits();
    compare_random<true>();
    compare_random<false>();
}
