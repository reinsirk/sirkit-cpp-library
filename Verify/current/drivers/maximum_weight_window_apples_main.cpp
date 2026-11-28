// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int n = 0;
    std::uint32_t duration = 0, width = 0;
    verify_in >> n >> duration >> width;
    std::vector<WeightedWindowPoint> points(n);
    for (auto &point : points)
    {
        verify_in >> point.x >> point.y;
        point.weight = 1;
    }
    // 正の座標と重みなので、負側にはみ出す窓を1から始めても得られる点は減らない。
    verify_out << maximum_weight_window(points, duration, width).weight << '\n';
}
