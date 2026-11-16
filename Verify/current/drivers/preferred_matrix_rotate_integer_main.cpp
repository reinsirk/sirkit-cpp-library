// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
#include <vector>

int main() {
    int rotation{}, scale{}, h{}, w{}; verify_in >> rotation >> scale >> h >> w;
    std::vector<std::vector<int>> image(h, std::vector<int>(w));
    for (auto& row : image) {
        std::string input; verify_in >> input;
        for (int x = 0; x < w; ++x) row[x] = input[x];
    }
    if (rotation == 90) image = rotate_90r(image);
    else if (rotation == 180) image = rotate_180(image);
    else if (rotation == 270) image = rotate_90l(image);
    for (const auto& row : image)
        for (int y = 0; y < scale; ++y) {
            for (const int pixel : row)
                for (int x = 0; x < scale; ++x) verify_out << static_cast<char>(pixel);
            verify_out << '\n';
        }
}
