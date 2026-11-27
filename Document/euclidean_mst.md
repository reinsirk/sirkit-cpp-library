# ユークリッド最小全域木

平面上の点を結ぶ最小全域木を求めます。辺の重みは2点間のユークリッド距離です。

```cpp
using Point = std::pair<std::int64_t, std::int64_t>;
std::vector<Point> points{{0, 0}, {2, 0}, {0, 1}};
auto tree = sirkit::euclidean_mst(points);
// 距離の二乗が1と4の2辺
```

## `euclidean_mst`

```text
std::vector<EuclideanMstEdge> euclidean_mst(std::span<const std::pair<std::int64_t, std::int64_t>> points);
```

各点は `{x, y}` の順に指定します。戻り値は `std::vector<EuclideanMstEdge>` で、各辺の `from` と `to` は入力の0始まり添字、`distance_squared` は距離の二乗です。辺の長さが必要なら各値の平方根を取ります。同じ座標の点は距離0の辺でつなぎます。

点が2個未満なら戻り値は空です。最小全域木が複数あるときは、そのうちの1つを返します。各軸の座標幅（最大座標と最小座標の差）が25,000を超える場合は `std::overflow_error` を送出します。

Delaunay三角形分割から候補辺を絞り、Kruskal法を適用します。点数を $N$ とすると $O(N\log(N+1))$ 時間・$O(N)$ 追加領域です。

[使用例](../Examples/euclidean_mst_example.cpp) ／ [Verify](../Verify/current/submissions/euclidean_mst.cpp)
