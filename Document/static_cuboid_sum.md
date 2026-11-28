# 疎な3次元の直方体和

重み付きの点を先にまとめて渡し、複数の直方体について内部の重みの和を求めます。領域は点と照会の件数に比例し、座標が広くても使えます。[本体](../Geometry/Static_Cuboid_Sum.cpp)

- `StaticCuboidSumPoint{x, y, z, weight}` で点を表します。同じ座標の点もそれぞれ加算します。
- `StaticCuboidSumQuery{lower, upper}` の `lower` と `upper` には、`{x, y, z}` の配列を渡します。各軸で下端を含み、上端を含まない半開区間です。
- `static_cuboid_sum(points, queries)` は照会順に総和を返します。幅が $0$ の軸があれば、その照会の答えは $0$ です。

座標と重みは符号付き64 bit整数です。総和を $2^{64}$ で割った余りを、符号付き64 bit整数に対応づけて返します。点数 $N$、照会数 $Q$ に対し、$O((N+Q)\log^2(N+Q+2))$ 時間、$O(N+Q)$ 領域です。

```cpp
const std::array points{sirkit::StaticCuboidSumPoint{1, 2, 3, 8},
                        sirkit::StaticCuboidSumPoint{2, 2, 3, 4}};
const std::array queries{sirkit::StaticCuboidSumQuery{{0, 0, 0}, {2, 3, 4}}};
assert(sirkit::static_cuboid_sum(points, queries)[0] == 8);
```

[実行できる使用例](../Examples/static_cuboid_sum_example.cpp)・[AtCoder Verify](../Verify/current/submissions/static_cuboid_sum_box_in_box.cpp)
