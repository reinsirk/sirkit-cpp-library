// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_CYCLIC_ROTATION_DEFINED
#include "../../../String/Cyclic_Rotation.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <random>
#include <string>
#include <string_view>

namespace
{
void compare(std::string_view text)
{
    const auto suffixes = std::string(text) + std::string(text);
    const auto rotations = std::string_view(suffixes);
    std::size_t smallest = 0;
    std::size_t largest = 0;
    for (std::size_t i = 1; i < text.size(); ++i)
    {
        const auto candidate = rotations.substr(i, text.size());
        if (candidate < rotations.substr(smallest, text.size()))
        {
            smallest = i;
        }
        if (candidate > rotations.substr(largest, text.size()))
        {
            largest = i;
        }
    }
    assert(sirkit::min_cyclic_rotation_index(text) == smallest);
    assert(sirkit::max_cyclic_rotation_index(text) == largest);
}
} // namespace

int main()
{
    compare("");
    compare("a");
    compare("aaaa");
    compare("abababab");
    for (std::size_t code = 0; code < 6561; ++code)
    {
        std::string text(8, '\0');
        auto value = code;
        for (auto &character : text)
        {
            character = static_cast<char>((value % 3) * 127);
            value /= 3;
        }
        compare(text);
    }

    std::mt19937 random(28498);
    for (int trial = 0; trial < 1500; ++trial)
    {
        std::string text(random() % 100, '\0');
        for (auto &character : text)
        {
            character = static_cast<char>(random() & 255U);
        }
        compare(text);
    }

    std::string sparse(1'000'000, 'a');
    sparse[567'890] = 'b';
    assert(sirkit::min_cyclic_rotation_index(sparse) == 567'891);
    assert(sirkit::max_cyclic_rotation_index(sparse) == 567'890);
}
