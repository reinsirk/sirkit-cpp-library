// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
    std::uint64_t ratio, n;
    std::size_t degree;
    verify_in >> ratio >> degree >> n;
    verify_out << sum_exponential_powers(ratio, degree, n) << '\n';
}
