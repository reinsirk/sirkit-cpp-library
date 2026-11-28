// SPDX-License-Identifier: CC0-1.0
#include <string>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    std::string a, b;
    verify_in >> a >> b;
    verify_out << regex_language_edit_distance(a, b) << '\n';
}
