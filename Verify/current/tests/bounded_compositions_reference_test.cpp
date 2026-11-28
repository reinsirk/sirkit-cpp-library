// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Bounded_Compositions.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

using Part = sirkit::BoundedCompositionPart;

static void check(const std::vector<Part> &bounds, std::size_t limit, std::uint32_t modulus)
{
    std::vector<std::uint32_t> expected(limit + 1);
    expected[0] = 1U % modulus;
    for (const auto &[lower, upper] : bounds)
    {
        std::vector<std::uint32_t> next(limit + 1);
        for (std::size_t old = 0; old <= limit; ++old)
        {
            for (auto part = lower; part <= std::min(upper, limit - old); ++part)
            {
                auto &cell = next[old + part];
                cell = static_cast<std::uint32_t>((std::uint64_t{cell} + expected[old]) % modulus);
            }
        }
        expected.swap(next);
    }
    assert(sirkit::bounded_composition_counts(bounds, limit, modulus) == expected);
    if (std::ranges::all_of(bounds, [](const Part &bound)
                            {
                                return bound.lower == 0;
                            }))
    {
        std::vector<std::size_t> upper_bounds;
        for (const auto &bound : bounds)
        {
            upper_bounds.push_back(bound.upper);
        }
        assert(sirkit::bounded_composition_counts(upper_bounds, limit, modulus) == expected);
    }
}

template <class Exception, class Test>
static void throws(const Test &test)
{
    bool caught = false;
    try
    {
        test();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

int main()
{
    check({}, 0, 1);
    check({}, 12, 100);
    check({{1, 2}}, 0, 17);
    check({{0, 0}, {0, 0}}, 0, 17);
    check({{0, 2}, {0, 2}, {0, 4}}, 9, 2);
    check({{1, 1}, {0, 19}, {2, 5}, {1, 4}}, 27, 3);
    constexpr auto kLargest = std::numeric_limits<std::size_t>::max();
    constexpr auto kMod = std::numeric_limits<std::uint32_t>::max();
    check({{kLargest, kLargest}, {1, 2}, {kLargest, kLargest}}, 8, kMod);
    check({{0, kLargest}, {1, kLargest}}, 20, kMod);
    check(std::vector<Part>(35, {0, 4}), 100, kMod);
    throws<std::invalid_argument>([]
                                  {
                                      const std::array<std::size_t, 1> bound{0};
                                      (void)sirkit::bounded_composition_counts(bound, 0, 0);
                                  });
    throws<std::invalid_argument>([]
                                  {
                                      const std::array bounds{Part{5, 8}, Part{1, 0}};
                                      (void)sirkit::bounded_composition_counts(bounds, 2);
                                  });
    throws<std::length_error>([&]
                              {
                                  const std::array<Part, 0> empty{};
                                  (void)sirkit::bounded_composition_counts(empty, kLargest);
                              });
    std::mt19937 rng(99222);
    for (int i = 0; i < 600; ++i)
    {
        std::vector<Part> bounds(rng() % 9);
        for (auto &[lower, upper] : bounds)
        {
            lower = (i % 2 == 0) ? 0 : rng() % 7;
            upper = lower + rng() % 12;
        }
        check(bounds, rng() % 45, rng() % 37 + 1);
    }
}
