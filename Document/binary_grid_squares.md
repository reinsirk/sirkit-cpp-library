# 二値グリッドの正方形

条件を満たすマスだけでできた正方形を数えます。最大の正方形の位置と、辺長ごとの個数も取得できます。

## `binary_grid_squares`

```cpp
template <typename IsFilled>
BinaryGridSquareStatistics binary_grid_squares(int height, int width, IsFilled filled);
```

各マスの採否を `filled(row, column)` で判定します。座標は0始まりで、各マスを1回ずつ調べます。呼び出し順序は行優先に限りません。

縦横を $H,W$ として $O(HW+1)$ 時間、$O(\min(H,W)+1)$ 領域です。縦横は非負で、検査が有効なら負の寸法には `std::invalid_argument`、`std::uint64_t` を超える総数には `std::overflow_error` を送出します。

## `BinaryGridSquareStatistics`

`count` は総数、`by_side[s]` は辺長 `s` の個数を表します。`by_side` の長さは $\min(H,W)+1$ で、`by_side[0]` は0です。

`largest` には最大の正方形が入り、1マスも選べなければ `std::nullopt` になります。

## `BinaryGridSquare`

`top`、`left` は左上の行と列、`side` は辺長です。最大辺長が同じなら、行が最小、次に列が最小のものを返します。

## 使用例

```cpp
const std::array<std::string_view, 3> grid{"...", "...", "#.."};
const auto result = sirkit::binary_grid_squares(3, 3, [&](int row, int col)
{
    return grid[row][col] == '.';
});
assert(result.count == 11 && result.by_side[2] == 3);
assert(result.largest && result.largest->side == 2);
```

[実装](../Algorithm/Binary_Grid_Squares.cpp)・[使用例の全体](../Examples/binary_grid_squares_example.cpp)・[Verify](../Verify/current/submissions/binary_grid_squares.cpp)
