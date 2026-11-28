// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Convolution_Wrap64.cpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

int main()
{
    using sirkit::convolution_mod_2_64;
    using Sequence = std::vector<std::uint64_t>;
    constexpr auto kHighest = std::numeric_limits<std::uint64_t>::max();
    assert(convolution_mod_2_64({}, {}).empty());
    assert(convolution_mod_2_64(Sequence{1}, {}).empty());
    assert((convolution_mod_2_64(Sequence{kHighest, 2}, Sequence{3, 4}) == Sequence{kHighest - 2, 2, 8}));

    std::mt19937_64 random(514813);
    for (const std::size_t n : {1, 4, 9, 32, 33, 42, 77, 129, 131, 273, 997})
    {
        const auto m = n + 23;
        Sequence a(n), b(m);
        for (auto &value : a)
        {
            value = random();
        }
        for (auto &value : b)
        {
            value = random();
        }
        Sequence naive(n + m - 1);
        for (std::size_t j = 0; j < m; ++j)
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                naive[i + j] += a[i] * b[j];
            }
        }
        assert(convolution_mod_2_64(a, b) == naive);
    }

    const Sequence large(4097, kHighest), smaller(1031, kHighest);
    const auto triangular = convolution_mod_2_64(large, smaller);
    for (std::size_t i = 0; i < triangular.size(); ++i)
    {
        assert(triangular[i] == std::min({i + 1, triangular.size() - i, smaller.size()}));
    }
    const Sequence too_large((std::size_t{1} << 21) + 1);
    for (const auto swapped : {false, true})
    {
        try
        {
            const Sequence unit{1};
            static_cast<void>(swapped ? convolution_mod_2_64(unit, too_large)
                                      : convolution_mod_2_64(too_large, unit));
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }
}
