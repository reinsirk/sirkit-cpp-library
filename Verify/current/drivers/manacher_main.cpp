// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
int main() {
    std::string s; verify_in >> s;
    const auto lengths = PalindromicLengths(s);
    if (!lengths.empty()) verify_out << lengths[0];
    for (std::size_t i = 1; i < lengths.size(); ++i) verify_out << ' ' << lengths[i];
    verify_out << '\n';
}
