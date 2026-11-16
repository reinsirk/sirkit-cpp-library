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
    std::vector<std::pair<Point, Point>> segments(n);
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
            int count = 0;
            for (const auto& [a,b] : segments) {
                const int first=sirkit::geo::orientation(p,q,a);
                const int second=sirkit::geo::orientation(p,q,b);
                count += first*second <= 0;
            }
            answer=std::max(answer,count);
        }
    }
    verify_out << answer << '\n';
}
