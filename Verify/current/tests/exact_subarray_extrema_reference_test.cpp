#include "../../../Algorithm/Exact_Subarray_Extrema.cpp"

#include <array>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <functional>
#include <limits>
#include <random>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace
{
using sirkit::ExactSubarrayExtremaResult;
using sirkit::analyze_subarrays_with_extrema;

template <typename Compare>
ExactSubarrayExtremaResult brute(const std::vector<int> &values, int minimum, int maximum, Compare compare)
{
    ExactSubarrayExtremaResult result;
    for (std::size_t begin = 0; begin < values.size(); ++begin)
    {
        int min_seen = values[begin], max_seen = values[begin];
        for (std::size_t end = begin; end < values.size(); ++end)
        {
            if (compare(values[end], min_seen))
            {
                min_seen = values[end];
            }
            if (compare(max_seen, values[end]))
            {
                max_seen = values[end];
            }
            if (min_seen == minimum && max_seen == maximum)
            {
                ++result.count;
                if (end + 1 - begin > result.right - result.left)
                {
                    result.left = begin;
                    result.right = end + 1;
                }
            }
        }
    }
    return result;
}

template <typename Compare>
void verify(const std::vector<int> &values, int minimum, int maximum, Compare compare)
{
    const auto expected = brute(values, minimum, maximum, compare);
    const auto actual = analyze_subarrays_with_extrema(values, minimum, maximum, compare);
    assert(actual.count == expected.count && actual.left == expected.left && actual.right == expected.right);
}

#ifndef SKIP_RUNTIME_CHECKS
template <typename Exception, typename Action>
void expect_exception(Action action)
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
    verify({}, 0, 1, std::less<>{});
    verify({3, 5, 1, -9, 3, 5, 1}, 1, 5, std::less<>{});
    verify({3, 5, 1, -9, 3, 5, 1}, 5, 1, std::greater<>{});
    const auto forward = analyze_subarrays_with_extrema(std::forward_list<int>{1, 2, 3, 1}, 1, 3);
    assert(forward.count == 4 && forward.left == 0 && forward.right == 4);

    const auto by_length = [](std::string_view left, std::string_view right) { return left.size() < right.size(); };
    const std::array words{std::string_view{"x"}, std::string_view{"yy"}, std::string_view{"abc"}, std::string_view{"z"}};
    const auto projected = analyze_subarrays_with_extrema(words, std::string_view{"a"}, std::string_view{"zzz"}, by_length);
    assert(projected.count == 4 && projected.left == 0 && projected.right == 4);

    std::mt19937 random(247500);
    for (int test = 0; test < 2200; ++test)
    {
        std::vector<int> values(random() % 43);
        for (auto &value : values)
        {
            value = static_cast<int>(random() % 19) - 9;
        }
        const auto a = static_cast<int>(random() % 21) - 10;
        const auto b = static_cast<int>(random() % 21) - 10;
        const int low = std::min(a, b), high = std::max(a, b);
        verify(values, low, high, std::less<>{});
        verify(values, high, low, std::greater<>{});
    }

    const std::vector<int> equal(100000, 7);
    const auto wide = analyze_subarrays_with_extrema(equal, 7, 7);
    assert(wide.count == 5000050000ULL && wide.left == 0 && wide.right == equal.size());
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::invalid_argument>([] {
        (void)analyze_subarrays_with_extrema(std::array{1, 2, 3}, 3, 1);
    });
    expect_exception<std::overflow_error>([] {
        (void)sirkit::detail::AddExactExtremaCount(std::numeric_limits<std::uint64_t>::max() - 1, 2);
    });
#endif
}
