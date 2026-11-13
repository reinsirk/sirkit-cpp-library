// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
int main() {
    std::string s; verify_in >> s;
    const auto sa = suffix_array(s);
    if (!sa.empty()) verify_out << sa[0];
    for (std::size_t i = 1; i < sa.size(); ++i) verify_out << ' ' << sa[i];
    verify_out << '\n';
}
