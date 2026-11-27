// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>

int main()
{
    std::string text, pattern;
    verify_in >> text >> pattern;
    for (auto index : find_occurrences(text, pattern))
    {
        verify_out << index << '\n';
    }
}
