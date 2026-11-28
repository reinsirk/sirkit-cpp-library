// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <cstdint>

int main()
{
    std::array<std::uint32_t, 3> shape{};
    verify_in >> shape[0] >> shape[1] >> shape[2];
    verify_out << sirkit::standard_young_tableau_count(shape) << '\n';
}
