# 凸多角形を弦で切った部分の面積・重心

境界順に並べた整数頂点を前処理し、2頂点を弦で結んで切り出した部分の面積と重心を求めます。座標の絶対値は $10^9$ 以下です。[本体](../Geometry/Polygon_Chord_Moments.cpp)

| 操作 | 内容 | 時間 |
| --- | --- | --- |
| `PolygonChordMoments(points)` | 凸多角形の頂点列を前処理する | $O(n)$ |
| `slice(from, to)` | `from` から入力順に `to` まで進み、弦で閉じた部分を求める | $O(1)$ |
| `total()` | 多角形全体の面積と重心を求める | $O(1)$ |

頂点は0始まりです。反時計回りの入力なら `slice` は弦を `from` から `to` に見た右側を返します。時計回りなら左側です。結果の `signed_area2` は符号付き面積の2倍で、`centroid` は `double` の座標対です。面積が0のときは `std::nullopt` になります。前処理領域は $O(n)$ です。

```cpp
const std::array<sirkit::PolygonChordMoments::Point, 4> square{
    {{0, 0}, {6, 0}, {6, 6}, {0, 6}}};
const sirkit::PolygonChordMoments polygon(square);
assert(polygon.slice(0, 2).centroid == (std::pair{4., 2.}));
```

[実行できる使用例](../Examples/polygon_chord_moments_example.cpp)・[AtCoder Verify](../Verify/current/submissions/polygon_chord_moments.cpp)
