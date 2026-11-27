// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>

int main()
{
    std::string text, pattern;
    verify_in >> text >> pattern;
    const auto matches = sirkit::wildcard_pattern_matching(text, pattern);
    std::string output;
    output.reserve(matches.size());
    for (const bool match : matches)
    {
        output.push_back(match ? '1' : '0');
    }
    verify_out << output << '\n';
}
