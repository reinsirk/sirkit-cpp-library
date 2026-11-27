# 凸包の層分解

点集合から凸包の境界上にある点をすべて取り除き、残った点に対して同じ操作を繰り返します。各点が取り除かれる回を調べると、外側から数えた層がわかります。

```cpp
using Point = std::pair<std::int64_t, std::int64_t>;
std::vector<Point> points{{0, 0}, {2, 0}, {2, 2}, {0, 2}, {1, 0}, {1, 1}};
auto layers = sirkit::convex_layers(points);
// {1, 1, 1, 1, 1, 2}
```

## `convex_layers`

```text
std::vector<std::size_t> convex_layers(std::span<const std::pair<std::int64_t, std::int64_t>> points);
```

座標は `std::pair<std::int64_t, std::int64_t>` の `{x, y}` です。戻り値は入力順の `std::vector<std::size_t>` で、最初の層を1とします。凸包の頂点と辺上の点は同じ回に取り除きます。同じ座標の点も同じ層です。

残った点がすべて一直線上にあれば、その回ですべて取り除きます。空入力の戻り値は空です。各軸の最大座標と最小座標の差が $10^6$ を超えると `std::overflow_error` を送出します。

上下の凸包を木に分けて管理し、点を除いたときは共通接線が変わりうる節だけを更新します。点数を $N$ とすると $O(N\log^2(N+1))$ 時間・$O(N)$ 追加領域です。

[使用例](../Examples/convex_layers_example.cpp) ／ [Verify](../Verify/current/submissions/convex_layers.cpp)
