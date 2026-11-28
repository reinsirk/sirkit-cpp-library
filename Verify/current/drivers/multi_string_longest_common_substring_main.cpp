// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <array>
#include <string>
#include <string_view>

int main()
{
    std::string first, second;
    verify_in >> first >> second;
    const std::array<std::string_view, 2> texts{first, second};
    const auto common = sirkit::longest_common_substring(texts);
    verify_out << common.starts[0] << ' ' << common.starts[0] + common.length << ' '
               << common.starts[1] << ' ' << common.starts[1] + common.length << '\n';
}
