// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>

int main()
{
    using Faces = std::array<int, 6>;
    const auto read = []
    {
        int top{}, south{}, east{}, west{}, north{}, bottom{};
        verify_in >> top >> south >> east >> west >> north >> bottom;
        return Faces{top, bottom, north, south, east, west};
    };
    const auto first = sirkit::canonical_dice_faces(read());
    const auto second = sirkit::canonical_dice_faces(read());
    verify_out << (first == second ? "Yes\n" : "No\n");
}
