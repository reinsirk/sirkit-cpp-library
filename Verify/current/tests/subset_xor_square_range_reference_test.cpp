// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Subset_Xor_Square_Range.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
std::uint32_t brute(std::span<const std::uint32_t> values, std::uint32_t mod)
{
    std::vector<std::uint32_t> xors{0};
    for (auto value : values)
    {
        const auto count = xors.size();
        for (std::size_t mask = 0; mask < count; ++mask)
        {
            xors.push_back(xors[mask] ^ value);
        }
    }
    std::uint64_t answer = 0;
    for (auto value : xors)
    {
        answer += std::uint64_t{value} * value % mod;
        answer %= mod;
    }
    return static_cast<std::uint32_t>(answer);
}

template <class Error, class Operation>
void expect_error(Operation operation)
{
    try
    {
        operation();
    }
    catch (const Error &)
    {
        return;
    }
    assert(false);
}
} // namespace

int main()
{
    constexpr std::uint32_t kMod = 998244353;
    std::mt19937 rng(0x06113792U);
    for (std::size_t n = 0; n <= 12; ++n)
    {
        std::vector<std::uint32_t> values(n);
        for (int trial = 0; trial < 32; ++trial)
        {
            for (auto &value : values)
            {
                value = static_cast<std::uint32_t>(rng()) & ((1U << (trial % 31)) - 1U);
            }
            sirkit::SubsetXorSquareRange<> query(values);
            for (int update = 0; update < 4; ++update)
            {
                if (n != 0)
                {
                    const auto index = rng() % n;
                    const auto value = static_cast<std::uint32_t>(rng()) & ((1U << (trial % 31)) - 1U);
                    query.set(index, value);
                    values[index] = value;
                }
                for (std::size_t left = 0; left <= n; ++left)
                {
                    for (std::size_t right = left; right <= n; ++right)
                    {
                        assert(query.sum_squares(left, right) == brute(std::span(values).subspan(left, right - left), kMod));
                    }
                }
            }
        }
    }
    const std::array<std::uint32_t, 5> high{0xFFFFFFFF, 0x80000000, 0x80000000, 0xAAAAAAAA, 0x7FFFFFFF};
    constexpr std::uint32_t kWideEvenMod = 4294967294U;
    sirkit::SubsetXorSquareRange<32, kWideEvenMod> large(high);
    for (std::size_t length = 0; length <= high.size(); ++length)
    {
        assert(large.sum_squares(0, length) == brute(std::span(high).first(length), kWideEvenMod));
    }
    const std::array<std::uint32_t, 2> same{1, 1};
    const sirkit::SubsetXorSquareRange<1, 1> mod_one(same);
    assert(mod_one.sum_squares(0, 2) == 0);
    sirkit::SubsetXorSquareRange<> query(same);
    expect_error<std::invalid_argument>([&]
                                        {
                                            query.set(0, 1U << 30);
                                        });
    assert(query.sum_squares(0, 2) == 2);
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)sirkit::SubsetXorSquareRange<>(high);
                                        });
    expect_error<std::out_of_range>([&]
                                    {
                                        query.set(2, 0);
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)query.sum_squares(3, 2);
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)query.sum_squares(0, 3);
                                    });
}
