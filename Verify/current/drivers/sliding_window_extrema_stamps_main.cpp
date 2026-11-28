// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

int main()
{
    std::size_t h{}, w{}, h_black{}, w_black{}, h_white{}, w_white{};
    verify_in >> h >> w >> h_black >> w_black >> h_white >> w_white;
    h_white = std::min(h_white, h_black);
    w_white = std::min(w_white, w_black);

    const std::size_t stride = w + 1;
    std::vector<std::int64_t> prefix((h + 1) * stride);
    for (std::size_t i = 1; i <= h; ++i)
    {
        std::int64_t row_sum = 0;
        for (std::size_t j = 1; j <= w; ++j)
        {
            std::int64_t value{};
            verify_in >> value;
            row_sum += value;
            prefix[i * stride + j] = prefix[(i - 1) * stride + j] + row_sum;
        }
    }
    const auto sum = [&](std::size_t y, std::size_t x, std::size_t dy, std::size_t dx) {
        const auto top = y * stride;
        const auto bottom = (y + dy) * stride;
        return prefix[bottom + x + dx] - prefix[bottom + x] - prefix[top + x + dx] + prefix[top + x];
    };

    std::vector white_sums(h - h_white + 1, std::vector<std::int64_t>(w - w_white + 1));
    for (std::size_t y = 0; y < white_sums.size(); ++y)
    {
        for (std::size_t x = 0; x < white_sums[y].size(); ++x)
        {
            white_sums[y][x] = sum(y, x, h_white, w_white);
        }
    }
    const auto worst = sirkit::sliding_window_extrema_2d(
        white_sums, h_black - h_white + 1, w_black - w_white + 1, std::greater<>{});
    std::int64_t score = 0;
    for (std::size_t y = 0; y < worst.size(); ++y)
    {
        for (std::size_t x = 0; x < worst[y].size(); ++x)
        {
            score = std::max(score, sum(y, x, h_black, w_black) - worst[y][x]);
        }
    }
    verify_out << score << '\n';
}
