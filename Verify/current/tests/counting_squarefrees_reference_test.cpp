// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Counting_Squarefrees.cpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

int main()
{
    constexpr unsigned kLimit = 3'500'000;
    std::vector<unsigned char> square_free(kLimit + 1, 1);
    square_free[0] = 0;
    for (unsigned d = 2; d * d <= kLimit; ++d)
    {
        for (unsigned multiple = d * d; multiple <= kLimit; multiple += d * d)
        {
            square_free[multiple] = 0;
        }
    }
    std::vector<unsigned> prefix(kLimit + 1);
    for (unsigned i = 1; i <= kLimit; ++i)
    {
        prefix[i] = prefix[i - 1] + square_free[i];
    }
    for (unsigned i = 0; i < 1000; ++i)
    {
        assert(sirkit::count_squarefrees(i) == prefix[i]);
    }
    std::mt19937 random(616037);
    for (int i = 0; i < 1400; ++i)
    {
        const unsigned n = random() % (kLimit + 1);
        assert(sirkit::count_squarefrees(n) == prefix[n]);
    }
    for (unsigned d = 2; d <= 20; ++d)
    {
        const unsigned threshold = d * d * d * d * d;
        for (unsigned n = threshold - 1; n <= threshold + 1; ++n)
        {
            assert(sirkit::count_squarefrees(n) == prefix[n]);
        }
    }
    // Pawlewicz, Counting Square-Free Numbers, Table 2.
    assert(sirkit::count_squarefrees(1'000'000'000'000ULL) == 607'927'102'274ULL);
    assert(sirkit::count_squarefrees(1'000'000'000'000'000'000ULL) == 607'927'101'854'022'750ULL);
    assert(sirkit::detail::squarefree_isqrt(std::numeric_limits<std::uint64_t>::max()) == (1ULL << 32) - 1);
}
