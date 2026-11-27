# ミンコフスキー和

凸多角形 $A$ と $B$ から、それぞれの図形上の点 $a$ と $b$ を足した点 $a+b$ 全体でできる凸多角形を求めます。たとえば、正方形と横向きの線分の和は、元の正方形より横に長い長方形になります。

```cpp
using Point = std::pair<std::int64_t, std::int64_t>;
std::vector<Point> square{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
std::vector<Point> segment{{0, 0}, {1, 0}};
auto polygon = sirkit::minkowski_sum(square, segment);
// (0, 0), (3, 0), (3, 2), (0, 2)
```

## `minkowski_sum`

```text
std::vector<std::pair<std::int64_t, std::int64_t>> minkowski_sum(
    std::span<const std::pair<std::int64_t, std::int64_t>> left,
    std::span<const std::pair<std::int64_t, std::int64_t>> right);
```

座標は `std::pair<std::int64_t, std::int64_t>` の `{x, y}` です。各図形の頂点を境界に沿って時計回りか反時計回りに渡します。始点の再掲、連続する同じ点、辺上の中間点を含められます。すべての点が同一直線上なら順序は任意です。

戻り値の頂点は $(y,x)$ が最小の頂点から反時計回りに並び、始点の再掲と辺上の中間点を含みません。和が点なら1頂点、線分なら両端を返し、一方でも空なら結果も空です。

各図形の座標幅には `std::uint32_t`、和の座標には `std::int64_t` の範囲が上限です。座標幅は、各軸の最大座標と最小座標の差です。範囲を超える場合は `std::overflow_error`、通常版で境界順の条件を満たさない場合は `std::invalid_argument` を送出します。

2つの図形の辺を方向順にたどります。頂点数を $N,M$ とすると、時間・追加領域ともに $O(N+M)$ です。

[使用例](../Examples/minkowski_sum_example.cpp) ／ [AtCoder提出用コード](../Verify/current/submissions/minkowski_sum.cpp) ／ [独立参照テスト](../Verify/current/tests/minkowski_sum_reference_test.cpp)
