// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <string>
int main() {
    std::string s; verify_in >> s;
    const auto sa = suffix_array(s);
    const auto lcp = lcp_array(s, sa);
    const auto n = static_cast<std::uint64_t>(s.size());
    auto answer = n * (n + 1) / 2;
    for (int length : lcp) answer -= static_cast<std::uint64_t>(length);
    verify_out << answer << '\n';
}
