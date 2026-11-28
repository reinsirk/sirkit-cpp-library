// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t n{};
    std::string first, second;
    verify_in >> n >> first >> second;
    verify_out << sirkit::count_cyclic_rotation_pairs(first, second) << '\n';
}
