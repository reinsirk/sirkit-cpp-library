// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>

int main()
{
    std::string text;
    verify_in >> text;
    verify_out << sirkit::min_palindromic_partition(text).size() - 1 << '\n';
}
