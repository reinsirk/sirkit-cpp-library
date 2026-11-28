// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>

int main()
{
    std::size_t n{};
    std::string text;
    verify_in >> n >> text;
    verify_out << sirkit::analyze_three_symbol_frequencies(text).all_distinct << '\n';
}
