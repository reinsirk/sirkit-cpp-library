// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>

// 反時計回りの凸多角形、整数座標|x|,|y|<=10000。2倍面積は正確な整数。
// O(N)時間・O(N)領域。
int main() {
    int n{}; verify_in >> n;
    std::vector<sirkit::geo::PointI> polygon(n);
    for (auto& point : polygon) verify_in >> point.x >> point.y;
    verify_out << static_cast<std::int64_t>(2*sirkit::geo::area(polygon)) << '\n';
}
