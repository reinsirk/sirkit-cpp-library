# 指定した和の長方形を数える

整数グリッドの中から、マスの合計が指定値になる長方形を数えます。

## `count_subrectangles_with_sum`

```cpp
template <typename ValueAt>
std::uint64_t count_subrectangles_with_sum(int height, int width, std::int64_t target,
                                         ValueAt value_at);
```

`value_at(row, col)` が0始まりの座標における整数値を返します。同じ座標への呼び出しは常に同じ値を返してください。幅・高さとも正の長方形を数え、グリッドの片方の寸法が0なら0です。

短い辺に沿って帯を選び、帯の中では長い辺方向の累積和を使います。$S=\min(H,W)$、$L=\max(H,W)$ とすると、`value_at` が $O(1)$ の場合は期待 $O(1+S^2L)$ 時間、$O(1+L)$ 領域です。

各帯の列ごとの和と累積和は `std::int64_t`、答えは `std::uint64_t` の範囲に収まる必要があります。検査が有効なら、負の寸法は `std::invalid_argument`、これらの計算範囲の超過は `std::overflow_error` になります。

## 使用例

```cpp
const int grid[2][2]{{1, -1}, {-1, 1}};
const auto count = sirkit::count_subrectangles_with_sum(2, 2, 0, [&](int row, int col)
{
    return grid[row][col];
});
assert(count == 5);
```

[実装](../Algorithm/Target_Sum_Subrectangles.cpp)・[使用例の全体](../Examples/target_sum_subrectangles_example.cpp)・[Verify](../Verify/current/submissions/target_sum_subrectangles.cpp)
