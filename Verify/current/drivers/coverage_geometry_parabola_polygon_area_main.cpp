// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <vector>

// 放物線を等幅8192分割した多角形。台形則の相対誤差1/8192^2、
// 公式範囲で絶対誤差<0.000020。一般の曲線への誤差保証ではない。
// O(8192)時間・領域。面積APIの浮動座標・長い点列を補足する。
int main() {
    int a{},b{}; verify_in >> a >> b;
    constexpr int kParts=8192;
    std::vector<sirkit::geo::RealPoint> polygon;
    polygon.reserve(kParts+1);
    for (int i=0; i<=kParts; ++i) {
        const long double x=a+static_cast<long double>(b-a)*i/kParts;
        polygon.push_back({x,(x-a)*(x-b)});
    }
    verify_out.fixed(12) << sirkit::geo::area(polygon) << '\n';
}
