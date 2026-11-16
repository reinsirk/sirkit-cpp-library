// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

// 最適な直線は2端点を通るものから選べる。閉じた交差条件のまま平行移動・回転する。
// N<=100。O(N^3)時間・O(N)領域。
int main() {
    int n{}; verify_in >> n;
    using Point = sirkit::geo::PointI;
    std::vector<sirkit::geo::Segment<std::int64_t>> segments(n);
    std::vector<Point> endpoints;
    for (auto& [a,b] : segments) {
        verify_in >> a.x >> a.y >> b.x >> b.y;
        endpoints.push_back(a); endpoints.push_back(b);
    }
    int answer = 0;
    for (std::size_t i=0; i<endpoints.size(); ++i) {
        for (std::size_t j=i+1; j<endpoints.size(); ++j) {
            const Point p=endpoints[i], q=endpoints[j];
            if (p == q) continue;
            // 入力端点は[-100,100]^2、整数方向の長さ>=1。
            // 入力の正方形内の交点はpから距離<283なので、この線分で直線を代用できる。
            const Point direction=q-p;
            const sirkit::geo::Segment<std::int64_t> line{p-direction*1000, p+direction*1000};
            int count = 0;
            for (const auto& [a,b] : segments) {
                count += sirkit::geo::intersects(line, sirkit::geo::Segment<std::int64_t>{a,b});
            }
            answer=std::max(answer,count);
        }
    }
    verify_out << answer << '\n';
}
