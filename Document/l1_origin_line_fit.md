# 原点を通る直線への近似

`l1_origin_line_fit(points)` は、整数点を原点を通る同一直線へ移すときのマンハッタン距離の総和を最小化します。点は `std::pair<std::int32_t, std::int32_t>` で渡します。

```cpp
auto fit = sirkit::l1_origin_line_fit(points);
// fit.cost は最小距離の和、fit.slope は直線の傾き。
```

`swapped` が `false` なら直線は `y = slope * x`、`true` なら `x = slope * y` です。最小距離の和と傾きは既約分数の `Fraction` で返ります。

各向きで傾きを `[-1, 1]` に絞り、座標の絶対値を重みにした中央値を求めます。時間は $O(n\log(n+1))$、領域は $O(n+1)$。最小距離の和の既約分子が `std::int64_t` に収まらない場合は例外です。

[使用例](../Examples/l1_origin_line_fit_example.cpp) ／ [Verify](../Verify/current/submissions/l1_origin_line_fit.cpp)
