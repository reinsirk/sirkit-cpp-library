// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>

int main()
{
    std::size_t queries{};
    verify_in >> queries;
    for (std::size_t i = 0; i < queries; ++i)
    {
        sirkit::GaussianInteger a, b;
        verify_in >> a.real >> a.imag >> b.real >> b.imag;
        const auto result = sirkit::gaussian_gcd(a, b);
        verify_out << result.real << ' ' << result.imag << '\n';
    }
}
