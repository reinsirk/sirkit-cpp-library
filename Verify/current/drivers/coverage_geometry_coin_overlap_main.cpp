// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

// 半径10。20角の同じ/隣接セルだけに重なる円がある。接している円は残す。
// 残る中心間距離>=20なので各セルの候補数は定数。O(N+1001^2)時間・領域。
int main() {
    int n{}; verify_in >> n;
    constexpr int kWidth=1001;
    std::vector<int> heads(kWidth*kWidth,-1), next;
    std::vector<sirkit::geo::Circle> circles;
    next.reserve(n); circles.reserve(n);
    for (int i=0; i<n; ++i) {
        int x{},y{}; verify_in >> x >> y;
        const sirkit::geo::Circle current{{static_cast<long double>(x),static_cast<long double>(y)},10};
        const int cx=x/20, cy=y/20;
        bool overlap=false;
        for (int a=std::max(0,cx-1); a<=std::min(kWidth-1,cx+1) && !overlap; ++a) {
            for (int b=std::max(0,cy-1); b<=std::min(kWidth-1,cy+1) && !overlap; ++b) {
                for (int j=heads[a*kWidth+b]; j!=-1; j=next[j]) {
                    const auto kind=sirkit::circle_intersection_kind(current,circles[j],sirkit::geo::Epsilon{0,0});
                    if (kind==sirkit::geo::CircleIntersectionKind::TwoPoints || kind==sirkit::geo::CircleIntersectionKind::Coincident) {
                        overlap=true; break;
                    }
                }
            }
        }
        if (!overlap) {
            const int cell=cx*kWidth+cy;
            next.push_back(heads[cell]); heads[cell]=static_cast<int>(circles.size());
            circles.push_back(current);
        }
    }
    verify_out << circles.size() << '\n';
}
