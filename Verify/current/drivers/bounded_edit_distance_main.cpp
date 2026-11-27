// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t limit;
    std::string a, b;
    verify_in >> limit >> a >> b;
    verify_out << (edit_distance_at_most(a, b, limit) ? "Yes" : "No") << '\n';
}
