# 重み付き中央値と絶対偏差

`WeightedL1Statistics(coordinates)` に、更新で使う座標を登録します。あらかじめ順位に変換する必要はありません。各座標の初期重みは `0` です。

| API | 内容 |
| --- | --- |
| `insert(x, w)` / `erase(x, w)` | 登録した `x` の重みを増やす / 減らす。足りない `erase` は `false`。 |
| `kth(k)` | 重みを個数とみなして、小さい方から `k` 番目の座標。0始まり。 |
| `minimizer_interval()` | $\sum_i w_i\lvert x-a_i\rvert$ が最小となる閉区間。重みの合計が0なら `nullopt`。 |
| `absolute_deviation(x)` | 任意の `x` に対する $\sum_i w_i\lvert x-a_i\rvert$。 |

```cpp
std::array<std::int64_t, 2> coordinates{2, 9};
sirkit::WeightedL1Statistics values(coordinates);
values.insert(2, 3);
values.insert(9, 2);
auto cost = values.absolute_deviation(2); // 14
```

登録した値が $U$ 個、異なる値が $D$ 個なら、初期化は $O(U\log(U+1))$、各操作は $O(\log(D+1))$ です。重みと距離の和は `std::uint64_t` の範囲で返ります。

[使用例](../Examples/weighted_l1_statistics_example.cpp) ／ [Verify](../Verify/current/submissions/weighted_l1_statistics_absolute_minima.cpp)
