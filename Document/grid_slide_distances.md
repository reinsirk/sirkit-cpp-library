# グリッドを任意の距離だけ直進する最短路

1手で同じ方向へ任意の距離を進めるとき、壁を避けて着くまでの最少手数を求めます。縦横移動と斜め移動に対応します。[本体](../Graph/Grid_Slide_Distances.cpp)

- `grid_slide_distance(grid, start, target, axes, wall = '#')` は指定した終点への手数を返します。
- `grid_slide_distances(grid, start, axes, wall = '#')` は全マスへの手数を返します。添字は `row * width + column` です。

グリッドは各行を `std::string` とし、座標は $0$ 始まりの `(行, 列)` です。`axes` の既定値は `GridSlideAxes::orthogonal` で縦横に移動します。斜め移動には `GridSlideAxes::diagonal` を指定します。壁と到達不能の値は `-1` です。

壁で区切られた直線を1つのノードとし、2つの直線が交わるマスを移れる場所として幅優先探索します。縦 $H$、横 $W$ に対し $O(HW)$ 時間・領域です。[移動距離に上限がある場合](grid_stride_distances.md)

## 使用例

```cpp
const std::array<std::string, 3> grid{"...", "...", "..."};
const auto distance = sirkit::grid_slide_distance(
    grid, {0, 0}, {2, 2}, sirkit::GridSlideAxes::diagonal);
// 斜めに1手。
```

[使用例](../Examples/grid_slide_distances_example.cpp)・[Verify](../Verify/current/submissions/grid_slide_distances.cpp)
