// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Maximum_Allowed_Pair.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using Index = std::pair<std::size_t, std::size_t>;
using Value = std::int64_t;

void expect_best(const std::optional<sirkit::MaximumAllowedPairResult> &result,
                 std::size_t left, std::size_t right, Value sum)
{
    assert(result && result->left == left && result->right == right && result->sum == sum);
}

#ifndef SKIP_RUNTIME_CHECKS
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
#endif

void boundary_cases()
{
    using sirkit::maximum_allowed_pair;
    constexpr Value minimum = std::numeric_limits<Value>::min();
    constexpr Value maximum = std::numeric_limits<Value>::max();
    const std::vector<Value> only_minimum{minimum}, only_maximum{maximum}, only_zero{0};
    assert(!maximum_allowed_pair({}, {}, {}));
    assert(!maximum_allowed_pair(only_minimum, {}, {}));
    assert(!maximum_allowed_pair({}, only_maximum, {}));
    expect_best(maximum_allowed_pair(only_minimum, only_maximum, {}), 0, 0, -1);
    expect_best(maximum_allowed_pair(only_minimum, only_zero, {}), 0, 0, minimum);
    expect_best(maximum_allowed_pair(only_maximum, only_zero, {}), 0, 0, maximum);

    const std::vector<Index> all{{0, 0}, {0, 0}};
    assert(!maximum_allowed_pair(only_maximum, only_maximum, all));
    const std::vector<Value> two{3, 2}, three{3, 2, 0};
    expect_best(maximum_allowed_pair(two, three, all), 0, 1, 5);
    expect_best(maximum_allowed_pair(three, two, all), 0, 1, 5);

    const std::vector<Value> mixed{minimum, 0};
    expect_best(maximum_allowed_pair(mixed, only_minimum, {}), 1, 0, minimum);
    expect_best(maximum_allowed_pair(only_minimum, mixed, {}), 0, 1, minimum);

#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<Index> bad_left{{1, 0}}, bad_right{{0, 1}};
    expect_error<std::out_of_range>([&] { static_cast<void>(maximum_allowed_pair(only_minimum, only_maximum, bad_left)); });
    expect_error<std::out_of_range>([&] { static_cast<void>(maximum_allowed_pair(only_minimum, only_maximum, bad_right)); });
    expect_error<std::out_of_range>([&] { static_cast<void>(maximum_allowed_pair({}, {}, all)); });
    expect_error<std::overflow_error>([&] { static_cast<void>(maximum_allowed_pair(only_minimum, only_minimum, {})); });
    expect_error<std::overflow_error>([&] { static_cast<void>(maximum_allowed_pair(only_maximum, only_maximum, {})); });
#endif
}

void random_cases()
{
    std::mt19937 random(0x45deb);
    for (int test = 0; test < 2600; ++test)
    {
        const std::size_t n = random() % 12, m = random() % 12;
        std::vector<Value> left(n), right(m);
        for (auto &value : left)
        {
            value = static_cast<Value>(random() % 25) - 12;
        }
        for (auto &value : right)
        {
            value = static_cast<Value>(random() % 25) - 12;
        }
        std::vector<Index> forbidden;
        std::vector<unsigned char> blocked(n * m);
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < m; ++j)
            {
                if ((random() % (1 + test % 7)) == 0)
                {
                    blocked[i * m + j] = 1;
                    forbidden.emplace_back(i, j);
                    if (random() % 2 == 0)
                    {
                        forbidden.emplace_back(i, j);
                    }
                }
            }
        }

        std::optional<Value> expected;
        Index first_maximum{0, 0};
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < m; ++j)
            {
                if (!blocked[i * m + j] && (!expected || *expected < left[i] + right[j]))
                {
                    expected = left[i] + right[j];
                    first_maximum = {i, j};
                }
            }
        }
        const auto result = sirkit::maximum_allowed_pair(left, right, forbidden);
        if (expected)
        {
            expect_best(result, first_maximum.first, first_maximum.second, *expected);
        }
        else
        {
            assert(!result);
        }
    }
}

int main()
{
    boundary_cases();
    random_cases();
}
