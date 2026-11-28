# 円板と多角形の共通部分の面積

`circle_polygon_intersection_area(vertices, radius, center)` は、単純多角形と円板が重なる面積を返します。頂点は周回順の `std::pair<double, double>` です。凹多角形と時計回りの入力にも対応します。中心を省くと原点を使います。[本体](../Geometry/Circle_Polygon_Intersection.cpp)

```cpp
const std::array<std::pair<double, double>, 4> right_half{
    {{0, -3}, {4, -3}, {4, 3}, {0, 3}}};
const double area = sirkit::circle_polygon_intersection_area(right_half, 2);
// 半径2の円板の半分なので 2π
```

各辺を円との交点で分け、線分と円弧の面積を加えます。頂点数 $n$ に対して $O(n)$ 時間、$O(1)$ 追加領域です。半径は非負で、中心からの座標差とともに有限かつ絶対値 $10^{140}$ 以下が前提です。

[実行できる使用例](../Examples/circle_polygon_intersection_example.cpp)・[AOJ Verify](../Verify/current/submissions/circle_polygon_intersection.cpp)
