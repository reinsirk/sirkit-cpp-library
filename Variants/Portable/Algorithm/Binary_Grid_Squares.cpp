// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Algorithm/Binary_Grid_Squares.cpp
// 参考（穴を避ける数え上げ）: https://atcoder.jp/contests/abc311/tasks/abc311_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/binary_grid_squares.md
//
// API一覧
// binary_grid_squares(height,width,filled): filled(row,column)が全て真の正方形を調べる。添字は0始まり。
// BinaryGridSquareStatistics{count,largest,by_side}: 総数、最大の正方形またはnullopt、辺長別の個数。
// by_side[s]は正の辺長sの個数。長さmin(height,width)+1でby_side[0]=0。
// BinaryGridSquare{top,left,side}: 左上の行・列と辺長。同じ最大辺長なら(top,left)の辞書順最小。
// O(height*width+1)時間、O(min(height,width)+1)領域。filledは各マスに1回、順序は指定しない。
// 縦横は非負。検査有効時は負のサイズ、uint64_tを超える総数を例外で通知。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BINARY_GRID_SQUARES_DEFINED
#define SIRKIT_BINARY_GRID_SQUARES_DEFINED

struct BinaryGridSquare
{
    int top;
    int left;
    int side;
};

struct BinaryGridSquareStatistics
{
    std::uint64_t count;
    std::optional<BinaryGridSquare> largest;
    std::vector<std::uint64_t> by_side;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::uint64_t AddBinaryGridSquareCounts(std::uint64_t a, std::uint64_t b)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (a > std::numeric_limits<std::uint64_t>::max() - b)
    {
        throw std::overflow_error("binary_grid_squares: too many squares");
    }
#endif
    return a + b;
}
} // namespace reinsirk::detail

template <typename IsFilled>
[[nodiscard]] BinaryGridSquareStatistics binary_grid_squares(int height, int width, IsFilled filled)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (height < 0 || width < 0)
    {
        throw std::invalid_argument("binary_grid_squares: negative dimension");
    }
#endif
    const bool transposed = height < width;
    const int rows = std::max(height, width);
    const int columns = std::min(height, width);
    const auto slots = static_cast<std::size_t>(columns) + 1;
    BinaryGridSquareStatistics answer{0, std::nullopt, std::vector<std::uint64_t>(slots)};
    if (columns == 0)
    {
        return answer;
    }

    // endingは各列の直近の行を保つ。行内を進める間、上と左上の値を退避する。
    std::vector<int> ending(slots);
    for (int i = 0; i < rows; ++i)
    {
        int upper_left = 0;
        for (int j = 0; j < columns; ++j)
        {
            const auto column = static_cast<std::size_t>(j) + 1;
            const int upper = ending[column];
            const auto [row, col] = transposed ? std::pair{j, i} : std::pair{i, j};
            int side = 0;
            if (std::invoke(filled, row, col))
            {
                side = 1 + std::min({upper_left, upper, ending[column - 1]});
                ++answer.by_side[static_cast<std::size_t>(side)];
                const BinaryGridSquare square{row - side + 1, col - side + 1, side};
                if (!answer.largest || side > answer.largest->side ||
                    (side == answer.largest->side &&
                     std::pair{square.top, square.left} < std::pair{answer.largest->top, answer.largest->left}))
                {
                    answer.largest = square;
                }
            }
            upper_left = upper;
            ending[column] = side;
        }
    }

    // 各右下隅は最大辺長以下の辺長へ一つずつ寄与する。
    std::uint64_t at_least = 0;
    for (int side = columns; side > 0; --side)
    {
        auto &frequency = answer.by_side[static_cast<std::size_t>(side)];
        at_least += frequency;
        frequency = at_least;
        answer.count = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AddBinaryGridSquareCounts(answer.count, at_least);
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
