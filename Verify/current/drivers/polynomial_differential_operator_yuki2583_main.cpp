// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t steps{}, length{};
    verify_in >> steps >> length;
    std::vector<std::uint32_t> multiplier(length);
    for (auto &coefficient : multiplier)
    {
        verify_in >> coefficient;
    }
    const auto answer = sirkit::polynomial_differential_operator_power(
        {0, 1}, {-1, 0, 1}, sirkit::FormalPowerSeries(std::move(multiplier)), steps, steps + 2);
    for (std::size_t i = 0; i < answer.size(); ++i)
    {
        verify_out << answer[i] << (i + 1 == answer.size() ? '\n' : ' ');
    }
}
