#include "../../../Algorithm/Inversion_Limited_Partitions.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
struct Mod11
{
    unsigned value = 0;
    Mod11 &operator+=(Mod11 other)
    {
        value = (value + other.value) % 11;
        return *this;
    }
    Mod11 &operator-=(Mod11 other)
    {
        value = (value + 11 - other.value) % 11;
        return *this;
    }
};

void verify(std::span<const std::int64_t> values, std::uint64_t limit)
{
    std::vector<std::vector<bool>> valid(values.size(), std::vector<bool>(values.size()));
    std::vector<std::size_t> expected_starts(values.size());
    for (std::size_t right = 0; right < values.size(); ++right)
    {
        expected_starts[right] = right;
        for (std::size_t left = right + 1; left != 0;)
        {
            --left;
            std::uint64_t inversions = 0;
            for (std::size_t i = left; i < right; ++i)
            {
                for (std::size_t j = i + 1; j <= right; ++j)
                {
                    inversions += values[i] > values[j];
                }
            }
            if (inversions <= limit)
            {
                valid[left][right] = true;
                expected_starts[right] = left;
            }
        }
    }
    assert(sirkit::inversion_limited_starts(values, limit) == expected_starts);

    std::uint64_t ways = values.empty() ? 1 : 0;
    if (!values.empty())
    {
        for (unsigned mask = 0; mask < (1U << (values.size() - 1)); ++mask)
        {
            std::size_t left = 0;
            bool possible = true;
            for (std::size_t right = 0; right + 1 < values.size(); ++right)
            {
                if (mask & (1U << right))
                {
                    possible &= valid[left][right];
                    left = right + 1;
                }
            }
            if (possible && valid[left].back())
            {
                ++ways;
            }
        }
    }
    assert(sirkit::count_inversion_limited_partitions(values, limit) == ways);
    assert(sirkit::count_inversion_limited_partitions<Mod11>(values, limit).value == ways % 11);
}
}

int main()
{
    verify({}, 0);
    verify(std::array<std::int64_t, 1>{7}, 0);
    verify(std::array<std::int64_t, 2>{7, 3}, 1);
    verify(std::array<std::int64_t, 5>{6, 4, 2, 2, 1}, 0);
    verify(std::array<std::int64_t, 5>{6, 4, 2, 2, 1}, 3);
    verify(std::array{
               std::numeric_limits<std::int64_t>::max(),
               std::numeric_limits<std::int64_t>::min(),
               std::numeric_limits<std::int64_t>::max(),
           },
           0);
    const std::array<std::int64_t, 64> zeros{};
    assert(sirkit::count_inversion_limited_partitions(zeros, 0) == (std::uint64_t{1} << 63));

    std::mt19937 random(89110);
    for (int trial = 0; trial < 1500; ++trial)
    {
        const auto n = static_cast<std::size_t>(random() % 15);
        std::vector<std::int64_t> values(n);
        for (auto &value : values)
        {
            value = static_cast<std::int64_t>(random() % 15) - 7;
        }
        const auto limit = static_cast<std::uint64_t>(random() % 20);
        verify(values, limit);
    }

    bool caught = false;
    try
    {
        sirkit::detail::CheckInversionLimitedSize(std::numeric_limits<int>::max());
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);
}
