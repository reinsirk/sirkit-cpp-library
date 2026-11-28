# グリッドを最大Kマスずつ進む最短路

上下左右のいずれかに一度で最大 $K$ マス進めるとき、壁を避けて着くまでの最少移動回数を求めます。[本体](../Graph/Grid_Stride_Distances.cpp)

- `grid_stride_distance(grid, start, target, k, wall = '#')` は指定した終点への回数を返します。
- `grid_stride_distances(grid, start, k, wall = '#')` は全マスへの回数を返します。添字は `row * width + column` です。

グリッドは同じ長さの `std::string` を行ごとに並べ、座標は $0$ 始まりの `(行, 列)` で指定します。一度の移動で壁をまたぐことはできません。壁や到達できないマスの値は `-1` です。

行と列ごとに訪問済みのマスを飛ばすので、各マスを高々1回キューに入れます。縦 $H$、横 $W$ に対し $O(HW\,\alpha(HW))$ 時間、$O(HW)$ 領域です。$\alpha$ は Union-Find の計算量に現れる逆アッカーマン関数です。

## 使用例

```cpp
const std::array<std::string, 2> grid{"...", ".#."};
const int steps = sirkit::grid_stride_distance(grid, {1, 0}, {1, 2}, 3);
// 真ん中に壁があるので、上の行を経由して3回。
```

[使用例](../Examples/grid_stride_distances_example.cpp)・[Verify](../Verify/current/submissions/grid_stride_distances.cpp)
