// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int n = 0;
    std::uint32_t side = 0;
    verify_in >> n >> side;
    std::vector<WeightedWindowPoint> points(n);
    for (auto &[x, y, weight] : points)
    {
        verify_in >> x >> y >> weight;
        x *= 2;
        y *= 2;
    }
    // 頂点間の開区間にも左下を置けるよう、半整数を整数に移す。
    verify_out << maximum_weight_window(points, 2 * side + 1, 2 * side + 1).weight << '\n';
}
