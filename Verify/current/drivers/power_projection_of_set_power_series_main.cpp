// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t variables{}, count{};
    verify_in >> variables >> count;
    std::vector<std::uint32_t> series(std::size_t{1} << variables), weights(series.size());
    for (auto &coefficient : series)
    {
        verify_in >> coefficient;
    }
    for (auto &weight : weights)
    {
        verify_in >> weight;
    }
    const auto projected = sirkit::set_series_power_projection(series, weights, count);
    for (std::size_t i = 0; i < projected.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << projected[i];
    }
    verify_out << '\n';
}
