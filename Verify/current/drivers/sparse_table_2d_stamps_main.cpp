// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>

int main()
{
    std::size_t h, w, black_height, black_width, white_height, white_width;
    verify_in >> h >> w >> black_height >> black_width >> white_height >> white_width;
    white_height = std::min(white_height, black_height);
    white_width = std::min(white_width, black_width);
    const std::size_t stride = w + 1;
    std::vector<std::int64_t> prefix((h + 1) * stride);
    for (std::size_t row = 0; row < h; ++row)
    {
        std::int64_t partial = 0;
        for (std::size_t col = 0; col < w; ++col)
        {
            std::int64_t a;
            verify_in >> a;
            partial += a;
            prefix[(row + 1) * stride + col + 1] = prefix[row * stride + col + 1] + partial;
        }
    }
    const auto sum = [&](std::size_t i, std::size_t j, std::size_t height, std::size_t width)
    {
        const auto end = (i + height) * stride;
        const auto start = i * stride;
        return prefix[end + j + width] - prefix[end + j] - prefix[start + j + width] + prefix[start + j];
    };
    const std::size_t candidate_rows = h - white_height + 1, candidate_cols = w - white_width + 1;
    std::vector<std::int64_t> candidates;
    candidates.reserve(candidate_rows * candidate_cols);
    for (std::size_t row = 0; row < candidate_rows; ++row)
    {
        for (std::size_t col = 0; col < candidate_cols; ++col)
        {
            candidates.push_back(sum(row, col, white_height, white_width));
        }
    }
    const auto maximum = [](std::int64_t lhs, std::int64_t rhs)
    {
        return std::max(lhs, rhs);
    };
    const sirkit::SparseTable2D table(candidate_rows, candidate_cols, std::span<const std::int64_t>(candidates), maximum);
    const std::size_t query_height = black_height - white_height + 1;
    const std::size_t query_width = black_width - white_width + 1;
    std::int64_t answer = 0;
    for (std::size_t row = 0; row + black_height <= h; ++row)
    {
        for (std::size_t col = 0; col + black_width <= w; ++col)
        {
            const auto taken = table.prod(row, col, row + query_height, col + query_width);
            answer = std::max(answer, sum(row, col, black_height, black_width) - taken);
        }
    }
    verify_out << answer << '\n';
}
