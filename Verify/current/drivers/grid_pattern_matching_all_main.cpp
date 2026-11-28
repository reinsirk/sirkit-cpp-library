// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>
#include <vector>

int main()
{
    std::size_t text_height{}, text_width{};
    verify_in >> text_height >> text_width;
    std::vector<std::string> text(text_height);
    for (auto &row : text)
    {
        verify_in >> row;
    }
    std::size_t pattern_height{}, pattern_width{};
    verify_in >> pattern_height >> pattern_width;
    std::vector<std::string> pattern(pattern_height);
    for (auto &row : pattern)
    {
        verify_in >> row;
    }
    for (const auto &[row, column] : sirkit::find_grid_pattern(text, pattern))
    {
        verify_out << row << ' ' << column << '\n';
    }
}
