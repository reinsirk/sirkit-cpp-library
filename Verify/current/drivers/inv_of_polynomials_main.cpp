// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t n{}, m{};
    verify_in >> n >> m;
    std::vector<std::uint32_t> value(n), modulus(m);
    for (auto &coefficient : value)
    {
        verify_in >> coefficient;
    }
    for (auto &coefficient : modulus)
    {
        verify_in >> coefficient;
    }
    const auto inverse = sirkit::polynomial_inverse_mod(sirkit::FormalPowerSeries(std::move(value)),
                                                        sirkit::FormalPowerSeries(std::move(modulus)));
    if (!inverse)
    {
        verify_out << "-1\n";
        return 0;
    }
    verify_out << inverse->size() << '\n';
    for (std::size_t i = 0; i < inverse->size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << (*inverse)[i];
    }
    verify_out << '\n';
}
