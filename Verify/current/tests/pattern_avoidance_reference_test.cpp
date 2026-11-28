// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Pattern_Avoidance.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

std::uint32_t power(std::uint64_t value, std::uint64_t exponent, std::uint32_t modulus)
{
    std::uint64_t answer = 1;
    while (exponent != 0)
    {
        if (exponent % 2 != 0)
        {
            answer = answer * value % modulus;
        }
        value = value * value % modulus;
        exponent /= 2;
    }
    return static_cast<std::uint32_t>(answer);
}

struct Field101
{
    std::uint32_t value = 0;

    Field101() = default;
    explicit Field101(std::uint32_t input) : value(input % 101)
    {
    }

    Field101 &operator+=(Field101 other)
    {
        value = (value + other.value) % 101;
        return *this;
    }

    Field101 &operator-=(Field101 other)
    {
        value = (value + 101 - other.value) % 101;
        return *this;
    }

    Field101 &operator*=(Field101 other)
    {
        value = value * other.value % 101;
        return *this;
    }

    friend Field101 operator-(Field101 a, Field101 b)
    {
        return a -= b;
    }
    friend Field101 operator*(Field101 a, Field101 b)
    {
        return a *= b;
    }
    friend Field101 operator/(Field101 a, Field101 b)
    {
        return a *= Field101(power(b.value, 99, 101));
    }
    friend bool operator==(Field101, Field101) = default;
};

std::uint32_t direct(std::size_t length, std::span<const std::string_view> forbidden, std::uint32_t modulus)
{
    std::size_t suffix_length = 0;
    for (const auto pattern : forbidden)
    {
        if (pattern.empty())
        {
            return 0;
        }
        suffix_length = std::max(suffix_length, pattern.size() - 1);
    }
    std::map<std::string, std::uint32_t> counts{{"", 1}};
    for (std::size_t at = 0; at < length; ++at)
    {
        std::map<std::string, std::uint32_t> next;
        for (const auto &[suffix, count] : counts)
        {
            for (char byte : std::string_view("ab"))
            {
                auto extended = suffix + byte;
                if (std::ranges::any_of(forbidden, [&](std::string_view pattern) { return extended.ends_with(pattern); }))
                {
                    continue;
                }
                if (extended.size() > suffix_length)
                {
                    extended.erase(0, extended.size() - suffix_length);
                }
                auto &cell = next[extended];
                cell = (cell + count) % modulus;
            }
        }
        counts.swap(next);
    }
    std::uint32_t total = 0;
    for (const auto &[suffix, count] : counts)
    {
        total = (total + count) % modulus;
    }
    return total;
}
}

int main()
{
    using std::string_view_literals::operator""sv;
    assert(sirkit::count_strings_avoiding_patterns<>(0, {}) == 1);
    assert(sirkit::count_strings_avoiding_patterns<>(1, {}) == 26);
    constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
    assert(sirkit::count_strings_avoiding_patterns<2>(maximum, {}) == power(2, maximum, kMod));
    const std::array empty{""sv};
    assert(sirkit::count_strings_avoiding_patterns<2>(0, empty) == 0);
    assert(sirkit::count_strings_avoiding_patterns<2>(100, empty) == 0);
    const std::array all_letters{"a"sv, "b"sv};
    assert(sirkit::count_strings_avoiding_patterns<2>(100, all_letters) == 0);
    const std::array longer{"bbbbbbbb"sv};
    assert(sirkit::count_strings_avoiding_patterns<2>(3, longer) == 8);
    const std::string byte(1, '\xff');
    const std::array<std::string_view, 1> high_bytes{byte};
    assert((sirkit::count_strings_avoiding_patterns<2, 254>(30, high_bytes) == 1));

    const std::string run(80, 'a');
    const std::array<std::string_view, 1> long_run{run};
    std::array<std::uint32_t, 401> avoids{};
    for (std::size_t length = 0; length < avoids.size(); ++length)
    {
        if (length < run.size())
        {
            avoids[length] = power(2, length, kMod);
        }
        else
        {
            for (std::size_t gap = 1; gap <= run.size(); ++gap)
            {
                avoids[length] = (avoids[length] + avoids[length - gap]) % kMod;
            }
        }
    }
    assert(sirkit::count_strings_avoiding_patterns<2>(80, long_run) == avoids[80]);
    assert(sirkit::count_strings_avoiding_patterns<2>(400, long_run) == avoids[400]);

    std::mt19937 gen(576326);
    for (int trial = 0; trial < 360; ++trial)
    {
        std::vector<std::string> patterns(gen() % 8);
        for (auto &pattern : patterns)
        {
            pattern.assign(gen() % 6 + 1, 'a');
            for (auto &byte : pattern)
            {
                byte = gen() % 2 == 0 ? 'a' : 'b';
            }
        }
        const std::vector<std::string_view> views(patterns.begin(), patterns.end());
        const auto length = gen() % 90;
        assert(sirkit::count_strings_avoiding_patterns<2>(length, views) == direct(length, views, kMod));
        assert((sirkit::count_strings_avoiding_patterns<2, 'a', Field101>(length, views).value == direct(length, views, 101)));
    }

#ifndef SKIP_RUNTIME_CHECKS
    bool outside_rejected = false;
    try
    {
        const std::array outside{"axb"sv};
        static_cast<void>(sirkit::count_strings_avoiding_patterns<2>(8, outside));
    }
    catch (const std::out_of_range &)
    {
        outside_rejected = true;
    }
    assert(outside_rejected);
#endif
}
