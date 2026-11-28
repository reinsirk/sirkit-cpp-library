// SPDX-License-Identifier: CC0-1.0
#include "../Math/Bounded_Compositions.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::array<sirkit::BoundedCompositionPart, 2> bounds{{{1, 2}, {0, 2}}};
    assert((sirkit::bounded_composition_counts(bounds, 4) == std::vector<std::uint32_t>{0, 1, 2, 2, 1}));
    const std::array<std::size_t, 2> upper{2, 2};
    assert((sirkit::bounded_composition_counts(upper, 4, 3) == std::vector<std::uint32_t>{1, 2, 0, 2, 1}));
}
