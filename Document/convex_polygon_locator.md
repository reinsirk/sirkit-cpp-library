# ConvexPolygonLocator

同じ凸多角形に対して、多数の点が内部・境界・外部のどこにあるかを調べます。頂点は辺に沿って並べます。時計回りと反時計回りのどちらでも構築でき、辺の途中の点や連続する同じ点が含まれていても使えます。

## 使い方

```cpp
using sirkit::geo::Location;
using sirkit::geo::PointI;
std::vector<PointI> polygon{{0, 0}, {4, 0}, {4, 3}, {0, 3}};
sirkit::geo::ConvexPolygonLocator locator(polygon);
assert(locator.locate({2, 1}) == Location::Inside);
assert(locator.locate({4, 1}) == Location::Boundary);
assert(locator.locate({5, 1}) == Location::Outside);
```

[使用例](../Examples/convex_polygon_locator_example.cpp)

空の凸多角形では `Outside` を返します。点や線分に退化した場合、その点・線分上は `Boundary` です。座標差とその積が `Point` の計算型に収まる整数座標を渡します。

## 計算量

頂点数を `n` とすると、構築は `O(n+1)` 時間と領域、各照会は `O(log(n+2))` 時間です。辺の並びが凸かどうかの検査は呼び出し側が担います。
