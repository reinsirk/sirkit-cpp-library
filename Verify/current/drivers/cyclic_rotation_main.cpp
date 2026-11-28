// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
#include <string_view>

int main()
{
    std::string input;
    verify_in >> input;
    const std::string_view text(input);
    const auto min_start = sirkit::min_cyclic_rotation_index(text);
    const auto max_start = sirkit::max_cyclic_rotation_index(text);
    verify_out << text.substr(min_start) << text.substr(0, min_start) << '\n'
               << text.substr(max_start) << text.substr(0, max_start) << '\n';
}
