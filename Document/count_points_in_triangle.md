# 三角形の内部にある点の個数

頂点の候補と数える対象点が固定なら、頂点を3つ選ぶたびに三角形の内部の点数を求められます。

```cpp
using Point = sirkit::CountPointsInTriangle::Point;
std::vector<Point> vertices{{0, 0}, {4, 0}, {0, 4}};
std::vector<Point> points{{1, 1}, {2, 2}, {0, 0}, {3, 3}};
sirkit::CountPointsInTriangle triangles(vertices, points);
auto answer = triangles.count(0, 1, 2);  // 1
```

## `CountPointsInTriangle`

```text
CountPointsInTriangle(std::span<const Point> vertices, std::span<const Point> points);
```

`CountPointsInTriangle::Point` は `std::pair<std::int64_t, std::int64_t>` で、各点を `{x, y}` で渡します。頂点候補がある場合、両方の点集合を合わせた各軸の座標幅（最大値と最小値の差）は $2\times 10^9$ 以下です。幅を超えると `std::overflow_error` を送出します。

頂点候補 $N$ 個と対象点 $M$ 個に対し、前処理は $O(N^2(M+1))$ 時間・$O(N^2)$ 領域です。頂点候補が空の場合は問い合わせを行えません。

## `count`

```text
std::size_t count(std::size_t a, std::size_t b, std::size_t c) const;
```

`vertices` の位置を0始まりで指定します。頂点の順序は任意で、面積0の三角形の答えは0です。頂点や辺の上の点を除き、同じ座標の対象点は出現回数分を数えます。通常版で添字が範囲外の場合は `std::out_of_range` を送出します。問い合わせ1回は $O(1)$ です。

[使用例](../Examples/count_points_in_triangle_example.cpp) ／ [Verify](../Verify/current/submissions/count_points_in_triangle.cpp)
