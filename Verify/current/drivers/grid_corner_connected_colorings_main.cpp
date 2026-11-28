// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int h = 0, w = 0;
    verify_in >> h >> w;
    verify_out << grid_corner_connected_colorings(h, w) << '\n';
}
