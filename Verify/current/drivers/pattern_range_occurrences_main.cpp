// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t queries{};
    std::string text, pattern;
    verify_in >> queries >> text >> pattern;
    const sirkit::PatternRangeOccurrences index(text, pattern);
    for (std::size_t i = 0; i < queries; ++i)
    {
        std::size_t left{}, right{};
        verify_in >> left >> right;
        verify_out << (index.count(left - 1, right) != 0 ? "Yes" : "No") << '\n';
    }
}
