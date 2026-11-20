# 閉線分の交差判定

`Geometry/Line_Segment.cpp`を貼り付ける。零長線分、端点の接触、区間の重なりを扱う。

座標は有限とし、必要な差と積は計算型`C`の範囲に収まるものとする。整数座標では許容幅を無視する。二進浮動小数点の計算型では、丸め誤差で向きや許容幅との比較が決まらない場合に、`C`へ変換した座標から積和を再計算する。

## `geo::intersects`

```text
template<class T, class C>
bool intersects(const Segment<T, C>& s, const Segment<T, C>& t, Epsilon eps = {});
```

閉線分 `s` と `t` が交わるときに `true` を返す。浮動座標では `eps` を向きと端点の判定に適用する。計算量は固定の座標型・計算型に対して O(1)。動的確保は行わない。

## `geo::intersection`

```text
template<class T, class C>
SegmentIntersection intersection(const Segment<T, C>& s, const Segment<T, C>& t, Epsilon eps = {});
```

戻り値の `kind` は `None`、`Point`、`Overlap` のいずれか。 `points` はそれぞれ空、交点1個、辞書順の両端点2個を持つ。点の型は `geo::RealPoint` で、交点の計算結果は `long double` の近似値。制約と `eps` の扱いは `intersects` と共通。計算量は固定の座標型・計算型に対して O(1)。

```cpp
geo::Segment<std::int64_t> a{{0, 0}, {4, 0}}, b{{2, -1}, {2, 1}};
bool hit = geo::intersects(a, b);
auto result = geo::intersection(a, b);
```

[AtCoder の検証問題](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_ah)と、[多倍長有理数によるローカル検査](../Verify/current/tests/coverage_geometry_segment_intersections_test.cpp)で検証する。
