// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>

int main()
{
    std::string text;
    verify_in >> text;
    const sirkit::SubstringIndex index(text);
    verify_out << index.distinct_substring_count() << '\n';
}
