// SPDX-License-Identifier: CC0-1.0
#include "../String/Multi_String_Longest_Common_Substring.cpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <string_view>

int main()
{
    const std::array<std::string_view, 3> texts{"banana", "kanata", "panama"};
    const auto common = sirkit::longest_common_substring(texts);
    assert(common.length == 3);
    for (std::size_t i = 0; i < texts.size(); ++i)
    {
        assert(texts[i].substr(common.starts[i], common.length) == "ana");
    }
}
