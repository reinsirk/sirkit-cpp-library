// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
    std::uint64_t ratio;
    std::size_t degree;
    verify_in >> ratio >> degree;
    verify_out << *sum_exponential_powers_limit(ratio, degree) << '\n';
}
