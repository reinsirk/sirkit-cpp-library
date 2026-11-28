# 多角形の格子点数

`lattice_polygon_points(vertices)` は、整数座標の頂点を持つ多角形について内部と境界の格子点を数えます。辺上の格子点を最大公約数で数え、内部の点数はPickの定理から求めます。[本体](../Geometry/Lattice_Polygon.cpp)

頂点を `std::pair<std::int64_t, std::int64_t>` の列で境界に沿って渡します。穴のない単純多角形を扱い、時計回りと反時計回りのどちらでも使えます。各軸の最大座標差は $2^{32}-1$ 以下です。空の入力や、全頂点が一直線上にある入力にも対応します。

結果の `interior` が内部、`boundary` が境界の点数です。`total()` は合計を返し、`std::uint64_t` の範囲を超えると `std::overflow_error` を送出します。頂点数 $N$、最大座標差 $W$ に対し、$O(N\log(W+2))$ 時間、$O(1)$ 追加領域です。

```cpp
using Point = std::pair<std::int64_t, std::int64_t>;
const std::array triangle{Point{0, 0}, Point{4, 0}, Point{2, 3}};
const auto counts = sirkit::lattice_polygon_points(triangle);
assert(counts.interior == 4 && counts.boundary == 6);
```

[実行できる使用例](../Examples/lattice_polygon_example.cpp)・[AtCoder Verify](../Verify/current/submissions/lattice_polygon_typical90.cpp)
