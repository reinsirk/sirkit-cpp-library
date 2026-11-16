// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
#include <vector>

int main() {
    int rotation{}, scale{}, h{}, w{}; verify_in >> rotation >> scale >> h >> w;
    std::vector<std::string> image(h);
    for (auto& row : image) verify_in >> row;
    if (rotation == 90) image = rotate_90r(image);
    else if (rotation == 180) image = rotate_180(image);
    else if (rotation == 270) image = rotate_90l(image);
    for (const auto& row : image)
        for (int y = 0; y < scale; ++y) {
            for (const char pixel : row)
                for (int x = 0; x < scale; ++x) verify_out << pixel;
            verify_out << '\n';
        }
}
