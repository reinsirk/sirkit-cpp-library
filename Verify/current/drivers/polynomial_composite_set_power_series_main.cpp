// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t degree{}, variables{};
    verify_in >> degree >> variables;
    std::vector<std::uint32_t> polynomial(degree), series(std::size_t{1} << variables);
    for (auto &coefficient : polynomial)
    {
        verify_in >> coefficient;
    }
    for (auto &coefficient : series)
    {
        verify_in >> coefficient;
    }
    const auto composed = sirkit::set_series_compose(polynomial, series);
    for (std::size_t i = 0; i < composed.size(); ++i)
    {
        verify_out << composed[i] << (i + 1 == composed.size() ? '\n' : ' ');
    }
}
