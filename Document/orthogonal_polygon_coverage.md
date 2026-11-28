# 直交多角形の重なり数

縦と横の辺で囲まれた多角形のうち、各格子マスの中心を含むものが何個あるかを一括で求めます。時計回りと反時計回りの頂点列を混ぜて渡せます。

## 使い方

```cpp
using sirkit::geo::OrthogonalPoint;
using sirkit::geo::OrthogonalPolygon;
std::vector<OrthogonalPolygon> polygons{
    {{0, 0}, {3, 0}, {3, 2}, {0, 2}},
    {{2, 1}, {2, 4}, {4, 4}, {4, 1}},
};
std::vector<OrthogonalPoint> cells{{0, 0}, {2, 1}, {3, 2}, {4, 1}};
auto counts = sirkit::geo::orthogonal_polygon_coverage(polygons, cells);
// [1, 2, 1, 0]
```

[使用例](../Examples/orthogonal_polygon_coverage_example.cpp)

`cells` の `{x,y}` は点 `(x+0.5,y+0.5)` を指定します。各多角形の辺は縦か横で、自分自身と交差しないことが前提です。異なる多角形同士は重なっていても使えます。

## 計算量

全頂点数を `p`、問い合わせ数を `q` とすると、時間は `O((p+q) log(p+q+2))`、追加領域は `O(p+q+1)` です。
