// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    std::uint64_t n;
    verify_in >> n;
    verify_out << summatory_totient(n) << '\n';
}
