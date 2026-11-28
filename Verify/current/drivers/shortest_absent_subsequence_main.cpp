// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>

int main()
{
    std::string text;
    verify_in >> text;
    verify_out << sirkit::shortest_absent_subsequence(text) << '\n';
}
