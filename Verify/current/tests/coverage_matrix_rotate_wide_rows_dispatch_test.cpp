// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/rotate.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

int main() {
    constexpr std::size_t width = 16384;
    for (std::size_t height : {64, 65}) {
        std::vector<std::vector<std::uint32_t>> input(height, std::vector<std::uint32_t>(width));
        for (std::size_t i = 0; i < height; ++i)
            for (std::size_t j = 0; j < width; ++j)
                input[i][j] = static_cast<std::uint32_t>(i * width + j);
        const auto rotated = sirkit::rotate_90r(input);
        assert(rotated.size() == width);
        for (const auto& row : rotated) assert(row.size() == height);
        for (std::size_t i = 0; i < height; ++i)
            for (std::size_t j = 0; j < width; ++j)
                assert(rotated[j][height - 1 - i] == i * width + j);
    }
}
