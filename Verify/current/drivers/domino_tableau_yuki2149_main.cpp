// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::uint32_t> shape(n);
    for (auto it = shape.rbegin(); it != shape.rend(); ++it)
    {
        verify_in >> *it;
    }
    verify_out << sirkit::domino_tableau_count(shape) << '\n';
}
