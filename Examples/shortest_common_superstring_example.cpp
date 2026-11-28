// SPDX-License-Identifier: CC0-1.0
#include "../String/Shortest_Common_Superstring.cpp"
#include <array>
#include <cassert>
#include <string_view>

int main()
{
    const std::array<std::string_view, 3> words{"snuke", "kensho", "uk"};
    const auto text = sirkit::shortest_common_superstring(words);
    assert(text == "snukensho");
}
