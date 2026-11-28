// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t degree{}, last_shift{};
    verify_in >> degree >> last_shift;
    std::vector<std::uint32_t> coefficients(degree + 1), weights(last_shift + 1);
    std::vector<std::int64_t> shifts(last_shift + 1);
    for (auto &coefficient : coefficients)
    {
        verify_in >> coefficient;
    }
    for (std::size_t i = 0; i <= last_shift; ++i)
    {
        shifts[i] = -static_cast<std::int64_t>(i);
        verify_in >> weights[i];
    }
    const auto result = sirkit::weighted_taylor_shifts(sirkit::FormalPowerSeries(std::move(coefficients)), shifts, weights);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
