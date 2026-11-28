// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <string>

int main()
{
    std::string text;
    verify_in >> text;
    const auto size = text.size();
    const sirkit::StringPeriods prefixes(text);
    const auto primitive = prefixes.primitive_root_length(size);
    if (primitive == size)
    {
        verify_out << "1\n1\n";
        return 0;
    }
    if (primitive == 1)
    {
        verify_out << size << "\n1\n";
        return 0;
    }

    std::ranges::reverse(text);
    const sirkit::StringPeriods suffixes(text);
    std::size_t ways = 0;
    for (std::size_t split = 1; split < size; ++split)
    {
        if (prefixes.primitive_root_length(split) == split &&
            suffixes.primitive_root_length(size - split) == size - split)
        {
            ++ways;
        }
    }
    verify_out << "2\n" << ways << '\n';
}
