// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <string>

int main()
{
    std::string text;
    std::uint64_t rank = 0;
    verify_in >> text >> rank;
    const sirkit::SubstringIndex index(text);
    verify_out << index.kth_distinct_substring(rank).value() << '\n';
}
