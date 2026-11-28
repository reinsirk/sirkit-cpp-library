// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Stirling_Columns.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

int main()
{
    constexpr std::uint32_t kMod = 998244353;
    assert(sirkit::stirling_first_column(2, 3).empty());
    assert(sirkit::stirling_second_column(2, 3).empty());
    assert((sirkit::stirling_first_column(0, 0) == std::vector<std::uint32_t>{1}));
    assert((sirkit::stirling_second_column(kMod - 1, kMod - 1) == std::vector<std::uint32_t>{1}));
    for (auto count : {&sirkit::stirling_first_column, &sirkit::stirling_second_column})
    {
        for (const auto maximum : {std::size_t{kMod}, sirkit::FormalPowerSeries::max_precision() + 1})
        {
            try
            {
                static_cast<void>(count(maximum, 1));
                assert(false);
            }
            catch (const std::length_error &)
            {
            }
        }
    }

    constexpr std::size_t kN = 310;
    constexpr std::size_t kK = 20;
    std::uint32_t first[kN + 1][kK + 1]{};
    std::uint32_t second[kN + 1][kK + 1]{};
    first[0][0] = second[0][0] = 1;
    for (std::size_t i = 1; i <= kN; ++i)
    {
        for (std::size_t j = 1; j <= kK; ++j)
        {
            first[i][j] = (first[i - 1][j - 1] + (kMod + 1 - i) * first[i - 1][j]) % kMod;
            second[i][j] = (second[i - 1][j - 1] + j * second[i - 1][j]) % kMod;
        }
    }
    for (std::size_t k = 0; k <= kK; ++k)
    {
        const auto left = sirkit::stirling_first_column(kN, k);
        const auto right = sirkit::stirling_second_column(kN, k);
        assert(left.size() == kN - k + 1 && right.size() == kN - k + 1);
        for (std::size_t n = k; n <= kN; ++n)
        {
            assert(left[n - k] == first[n][k]);
            assert(right[n - k] == second[n][k]);
        }
    }
}
