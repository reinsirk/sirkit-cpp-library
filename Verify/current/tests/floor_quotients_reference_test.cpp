// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_FLOOR_QUOTIENTS_TEST_SOURCE
#define SIRKIT_FLOOR_QUOTIENTS_TEST_SOURCE "../../../Math/Floor_Quotients.cpp"
#endif
#include SIRKIT_FLOOR_QUOTIENTS_TEST_SOURCE
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <set>
#include <vector>

int main()
{
    for (std::uint64_t n = 0; n <= 600; ++n)
    {
        std::set<std::uint64_t> reference;
        for (std::uint64_t i = 1; i <= n; ++i)
        {
            reference.insert(n / i);
        }
        const auto result = sirkit::enumerate_quotients(n);
        assert(std::ranges::equal(result, reference));

        std::uint64_t current = 1;
        std::size_t blocks = 0;
        sirkit::for_each_floor_quotient(n, [&](auto first, auto last, auto quotient)
                                        {
                                            assert(first == current && first <= last && last <= n);
                                            for (auto i = first; i <= last; ++i)
                                            {
                                                assert(n / i == quotient);
                                            }
                                            assert(last == n || n / (last + 1) < quotient);
                                            current = last + 1;
                                            ++blocks;
                                        });
        assert(current == n + 1);
        assert(blocks == reference.size());
    }

    constexpr std::uint64_t kSquare = 100000;
    assert(sirkit::enumerate_quotients(kSquare * kSquare).size() == kSquare * 2 - 1);
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    struct Stop
    {
    };
    try
    {
        std::uint64_t calls = 0;
        sirkit::for_each_floor_quotient(kMax, [&](auto first, auto last, auto q)
                                        {
                                            ++calls;
                                            assert(first == calls && last == calls && q == kMax / first);
                                            if (calls == 10)
                                            {
                                                throw Stop{};
                                            }
                                        });
        assert(false);
    }
    catch (const Stop &)
    {
    }
}
