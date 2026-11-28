# 半平面・凸多角形の共通部分

`HalfPlane{a, b, c}` は境界を含む領域 $ax+by\le c$ を表します。`half_plane_intersection(planes, lower, upper)` は指定した矩形内で、すべての半平面に含まれる領域を求めます。矩形の2点は左下・右上の順です。[本体](../Geometry/Half_Plane_Intersection.cpp)

```cpp
const std::array planes{
    sirkit::HalfPlane{-1, 0, 0},
    sirkit::HalfPlane{0, -1, 0},
    sirkit::HalfPlane{1, 1, 3},
};
const auto triangle = sirkit::half_plane_intersection(planes, {-10, -10}, {10, 10});
assert(triangle.size() == 3);
```

`convex_polygon_intersection(p, q)` は2つの凸多角形を交差させます。入力は面積が正で、整数頂点を反時計回りに並べます。結果の頂点は反時計回りで、線分なら2点、点なら1点、空領域なら空列です。

$h$ 個の半平面で $O(h\log(h+1))$ 時間、$O(h+1)$ 領域を使います。矩形と多角形の座標は絶対値 $10^9$ 以下、係数は $|a|,|b|\le 2\cdot10^9$、$|c|\le 2\cdot10^{18}$ です。内外判定は整数で行い、返す頂点を `double` に丸めます。

[実行できる使用例](../Examples/half_plane_intersection_example.cpp)・[AtCoder Verify](../Verify/current/submissions/half_plane_intersection.cpp)
