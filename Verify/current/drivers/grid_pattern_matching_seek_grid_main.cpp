// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>
#include <vector>

int main()
{
    std::size_t text_size{}, pattern_size{};
    verify_in >> text_size >> pattern_size;
    std::vector<std::string> text(text_size), pattern(pattern_size);
    for (auto &row : text)
    {
        verify_in >> row;
    }
    for (auto &row : pattern)
    {
        verify_in >> row;
    }
    const auto matches = sirkit::find_grid_pattern(text, pattern);
    const auto [row, column] = matches.front();
    verify_out << row + 1 << ' ' << column + 1 << '\n';
}
