# 3次元整数座標の幾何

`Point3i{x, y, z}` は整数の点です。座標は $-10^9$ 以上 $10^9$ 以下です。

| API | 内容 |
| --- | --- |
| `orientation_3d(a, b, c, p)` | 法線 $(b-a)\times(c-a)$ が向く側なら `1`、反対側なら `-1`、同一平面または共線なら `0`。 |
| `triangular_prism_position(a, b, c, p)` | `p` を平面に垂直に下ろし、三角形の内部・境界・外部を判定する。`a, b, c` が共線なら `kDegenerate`。 |
| `signed_plane_distance(a, b, c, p)` | 法線方向を正とする符号付き距離。共線なら `nullopt`。 |

```cpp
sirkit::Point3i a{0, 0, 0}, b{3, 0, 0}, c{0, 3, 0}, p{1, 1, 2};
auto distance = sirkit::signed_plane_distance(a, b, c, p); // 2
```

いずれも $O(1)$ 時間・領域です。向きと内外を整数演算で判定します。距離だけは `double` で返します。

[使用例](../Examples/integer_geometry_3d_example.cpp) ／ [三角柱のVerify](../Verify/current/submissions/triangular_prism_position.cpp) ／ [距離のVerify](../Verify/current/submissions/point_plane_distance_sum.cpp)
