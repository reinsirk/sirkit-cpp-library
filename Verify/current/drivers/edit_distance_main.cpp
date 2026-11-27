// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>

int main()
{
    std::string a, b;
    verify_in >> a >> b;
    verify_out << edit_distance(a, b) << '\n';
}
