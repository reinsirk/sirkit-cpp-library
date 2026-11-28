// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    std::size_t tile_count{}, height{}, width{};
    verify_in >> tile_count >> height >> width;
    sirkit::ExactCover cover(height * width, tile_count);
    for (std::size_t tile = 0; tile < tile_count; ++tile)
    {
        std::size_t h{}, w{};
        verify_in >> h >> w;
        for (int orientation = 0; orientation < (h == w ? 1 : 2); ++orientation)
        {
            for (std::size_t top = 0; top + h <= height; ++top)
            {
                for (std::size_t left = 0; left + w <= width; ++left)
                {
                    std::vector<std::size_t> columns;
                    columns.reserve(h * w + 1);
                    for (auto row = top; row < top + h; ++row)
                    {
                        for (auto column = left; column < left + w; ++column)
                        {
                            columns.push_back(row * width + column);
                        }
                    }
                    columns.push_back(height * width + tile);
                    cover.add_row(columns);
                }
            }
            std::swap(h, w);
        }
    }
    verify_out << (cover.solve() ? "Yes\n" : "No\n");
}
