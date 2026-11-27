// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t degree{}, count{};
    std::uint64_t first{};
    verify_in >> degree >> first >> count;
    std::vector<std::uint32_t> initial(degree), coefficients(degree);
    for (auto &value : initial)
    {
        verify_in >> value;
    }
    for (auto &value : coefficients)
    {
        verify_in >> value;
    }
    const auto result = sirkit::consecutive_linear_recurrence(initial, coefficients, first, count);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_out << result[i] << (i + 1 == result.size() ? '\n' : ' ');
    }
}
