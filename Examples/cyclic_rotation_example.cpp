// SPDX-License-Identifier: CC0-1.0
#include "../String/Cyclic_Rotation.cpp"
#include <cassert>
#include <string>

int main()
{
    const std::string text = "aaba";
    const auto start = sirkit::min_cyclic_rotation_index(text);
    assert(start == 3);
    assert(text.substr(start) + text.substr(0, start) == "aaab");
    assert(sirkit::max_cyclic_rotation_index(text) == 2);
}
