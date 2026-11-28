# 区間に点を置いて重みを最大化

整数座標 `0..n-1` から好きな点を選びます。選んだ点が一つでも入る区間の重みを一度ずつ加え、その合計を最大にします。点を一つも選ばない場合の合計は0です。

## `StabbingInterval`

`{left, right, weight}` で半開区間 `[left, right)` と重みを指定します。同じ区間を複数回指定した場合は、それぞれの重みを足します。空区間の重みは加算しません。

## `IntervalStabbingResult`

`weight` は最大の合計、`points` はそれを実現する点の昇順の列です。

## `maximize_interval_stabbing_weight(n, intervals)`

区間の右端を左から順に処理し、最後に選んだ点ごとの最良値を遅延セグメント木で管理します。区間が $m$ 個なら $O(1+(n+m)\log(n+2))$ 時間、$O(n+m+1)$ 領域です。

`0 <= left <= right <= n`、`0 <= n < INT_MAX` が必要です。途中の得点と遅延更新の累積値も `std::int64_t` に収めてください。

## 使用例

```cpp
const std::array<sirkit::StabbingInterval, 3> intervals{{
    {0, 3, 10},
    {1, 4, -10},
    {2, 5, 10},
}};
const auto result = sirkit::maximize_interval_stabbing_weight(5, intervals);
assert(result.weight == 20);
assert((result.points == std::vector<int>{0, 4}));
```

[実装](../Algorithm/Weighted_Interval_Stabbing.cpp)・[使用例の全体](../Examples/weighted_interval_stabbing_example.cpp)・[Verify](../Verify/current/submissions/weighted_interval_stabbing.cpp)
