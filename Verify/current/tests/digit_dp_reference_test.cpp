// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Digit_DP.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

static std::string numeral(unsigned number, unsigned radix, bool lowercase)
{
    const std::string_view alphabet = lowercase ? "0123456789abcdefghijklmnopqrstuvwxyz" : "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string digits;
    do
    {
        digits.push_back(alphabet[number % radix]);
        number /= radix;
    } while (number != 0);
    std::reverse(digits.begin(), digits.end());
    return digits;
}

static void check(unsigned bound, unsigned radix, std::size_t states, std::size_t initial, bool prefix_zeros)
{
    const auto step = [=](std::size_t state, unsigned digit, bool leading) -> std::optional<std::size_t>
    {
        if (!leading && (digit + state) % 7 == 4)
        {
            return std::nullopt;
        }
        if (leading)
        {
            return (state + 1) % states;
        }
        return (state * radix + digit + 1) % states;
    };
    const auto upper = std::string(prefix_zeros ? 2 : 0, '0') + numeral(bound, radix, prefix_zeros);
    std::vector<std::uint64_t> expected(states);
    for (unsigned number = 0; number <= bound; ++number)
    {
        std::vector<unsigned> digits(upper.size());
        auto x = number;
        for (auto it = digits.rbegin(); it != digits.rend(); ++it)
        {
            *it = x % radix;
            x /= radix;
        }
        std::optional<std::size_t> state(initial);
        bool started = false;
        for (const auto digit : digits)
        {
            started |= digit != 0;
            state = step(*state, digit, !started);
            if (!state)
            {
                break;
            }
        }
        if (state)
        {
            ++expected[*state];
        }
    }
    assert(sirkit::digit_dp_counts<std::uint64_t>(upper, states, initial, step, radix) == expected);
}

template <class Error, class Test>
static void expect_error(const Test &test)
{
    bool thrown = false;
    try
    {
        test();
    }
    catch (const Error &)
    {
        thrown = true;
    }
    assert(thrown);
}

int main()
{
    check(0, 2, 1, 0, false);
    check(701, 10, 1, 0, false);
    check(35, 36, 2, 0, false);
    check(35, 36, 2, 0, true);
    check(1023, 16, 11, 3, true);
    std::mt19937 rng(78210);
    for (int test = 0; test < 220; ++test)
    {
        const std::size_t states = 1 + rng() % 21;
        check(rng() % 401, 2 + rng() % 35, states, rng() % states, test % 3 == 0);
    }
    const auto no_digits = [](std::size_t, unsigned, bool) -> std::optional<std::size_t>
    {
        return std::nullopt;
    };
    assert(sirkit::digit_dp_counts<std::uint64_t>("701", 1, 0, no_digits)[0] == 0);
    const auto identity = [](std::size_t state, unsigned, bool)
    {
        return state;
    };
    const auto invalid = [&](const std::string &digits, std::size_t states, std::size_t initial, unsigned radix)
    {
        return sirkit::digit_dp_counts<std::uint64_t>(digits, states, initial, identity, radix);
    };
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)invalid("", 1, 0, 10);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)invalid("0", 0, 0, 10);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)invalid("0", 1, 0, 1);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)invalid("0", 1, 0, 37);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)invalid("a", 1, 0, 10);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)invalid("1/", 1, 0, 10);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)invalid("[", 1, 0, 36);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)invalid("{", 1, 0, 36);
                                        });
    expect_error<std::length_error>([&]
                                    {
                                        (void)invalid("0", std::numeric_limits<std::size_t>::max(), 0, 10);
                                    });
    expect_error<std::out_of_range>([]
                                    {
                                        (void)sirkit::digit_dp_counts<std::uint64_t>("2", 1, 0, [](std::size_t, unsigned, bool)
                                                                                     {
                                                                                         return 1U;
                                                                                     });
                                    });
}
