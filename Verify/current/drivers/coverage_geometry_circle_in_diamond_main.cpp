// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <array>
#include <cstdint>
#include <vector>

// 中心が内部、かつ全4辺への距離が半径より大きいことが必要十分。接触は不可。
// この問題の座標・半径では答え<512。O(log 512)時間・O(1)領域。
int main() {
    sirkit::geo::PointI center; std::int64_t radius{};
    verify_in >> center.x >> center.y >> radius;
    std::vector<sirkit::geo::PointI> polygon(4);
    auto fits = [&](std::int64_t d) {
        polygon={{d,0},{0,d},{-d,0},{0,-d}};
        if (sirkit::geo::contains(polygon,center) != sirkit::geo::Location::Inside) return false;
        for (int i=0; i<4; ++i) {
            const sirkit::geo::Segment<std::int64_t> side{polygon[i],polygon[(i+1)%4]};
            if (sirkit::geo::distance(center,side) <= radius) return false;
        }
        return true;
    };
    std::int64_t low=0, high=512;
    while (high-low>1) {
        const auto middle=(low+high)/2;
        if (fits(middle)) high=middle; else low=middle;
    }
    verify_out << high << '\n';
}
