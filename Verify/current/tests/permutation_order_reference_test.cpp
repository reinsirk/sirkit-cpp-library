// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Permutation_Order.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

int main()
{
    const sirkit::fenwick_tree<std::uint32_t> frequencies(std::vector<std::uint32_t>{1, 2, 0, 5});
    assert(frequencies.lower_bound(3) == 1);
    assert(frequencies.upper_bound(3) == 3);

    for (int n = 0; n <= 7; ++n)
    {
        std::vector<int> values(n);
        std::iota(values.begin(), values.end(), 0);
        std::uint64_t rank = 0;
        do
        {
            const auto code = sirkit::permutation_lehmer_code(values);
            for (int i = 0; i < n; ++i)
            {
                const auto expected = std::count_if(values.begin() + i + 1, values.end(),
                                                    [&](int value)
                                                    {
                                                        return value < values[i];
                                                    });
                assert(code[i] == expected);
            }
            assert(sirkit::permutation_from_lehmer_code(code) == values);
            assert(sirkit::permutation_rank(values) == rank);
            assert(sirkit::permutation_unrank(n, rank) == values);
            ++rank;
        } while (std::next_permutation(values.begin(), values.end()));
        assert(!sirkit::permutation_unrank(n, rank));
    }

    constexpr auto kMaximum = std::numeric_limits<std::uint64_t>::max();
    auto boundary = sirkit::permutation_unrank(21, kMaximum);
    assert(boundary && sirkit::permutation_rank(*boundary) == kMaximum);
    assert(std::next_permutation(boundary->begin(), boundary->end()) && !sirkit::permutation_rank(*boundary));

    const int length = 100000;
    std::vector<int> descending(length);
    std::iota(descending.rbegin(), descending.rend(), 0);
    const auto reversed = sirkit::permutation_lehmer_code(descending);
    assert(reversed == descending && sirkit::permutation_from_lehmer_code(reversed) == descending);
    assert(!sirkit::permutation_rank(descending));

    auto invalid = [](auto action)
    {
        try
        {
            action();
        }
        catch (const std::invalid_argument &)
        {
            return true;
        }
        return false;
    };
    assert(invalid([]
                   {
                       (void)sirkit::permutation_unrank(-1, 0);
                   }));
#ifndef SKIP_RUNTIME_CHECKS
    assert(invalid([]
                   {
                       const std::array<int, 3> values{0, -1, 2};
                       (void)sirkit::permutation_lehmer_code(values);
                   }));
    assert(invalid([]
                   {
                       const std::array<int, 3> values{0, 3, 2};
                       (void)sirkit::permutation_lehmer_code(values);
                   }));
    assert(invalid([]
                   {
                       const std::array<int, 3> values{0, 1, 1};
                       (void)sirkit::permutation_rank(values);
                   }));
    assert(invalid([]
                   {
                       const std::array<int, 3> code{2, -1, 0};
                       (void)sirkit::permutation_from_lehmer_code(code);
                   }));
    assert(invalid([]
                   {
                       const std::array<int, 3> code{2, 0, 1};
                       (void)sirkit::permutation_from_lehmer_code(code);
                   }));
#endif
}
