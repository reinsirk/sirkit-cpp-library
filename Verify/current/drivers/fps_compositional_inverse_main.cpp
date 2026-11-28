// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t size{};
    verify_in >> size;
    std::vector<std::uint32_t> coefficients(size);
    for (auto &value : coefficients)
    {
        verify_in >> value;
    }
    const auto inverse = sirkit::fps_compositional_inverse(sirkit::FormalPowerSeries(std::move(coefficients)), size);
    for (std::size_t i = 0; i < size; ++i)
    {
        verify_out << inverse[i] << (i + 1 == size ? '\n' : ' ');
    }
}
