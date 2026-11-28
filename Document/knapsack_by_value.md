# 価値で添字を取る0/1ナップサック

各品物を1回まで選びます。容量が大きく、価値の合計が小さいときに使えます。各価値に必要な最小重量を前計算し、複数の容量を照会できます。

| API | 操作・計算量 |
|---|---|
| `KnapsackByValue(items)` | `Item{weight, value}` の列を前計算します。品物が $n$ 個、価値の合計が $V$ なら $O((n+1)(V+1))$ 時間、$O(V+1)$ 領域です。 |
| `minimum_weight(value)` | 指定した価値をちょうど作る最小重量。選び方がないか、最小重量が `std::uint64_t` に収まらなければ `std::nullopt`。$O(1)$。 |
| `max_value(budget)` | 指定した容量に収まる最大価値。$O(\log(V+2))$。 |

`weight`・`budget` は `std::uint64_t`、`value` は `std::size_t` です。重量0や価値0の品物も扱えます。何も選ばないと価値・重量は0です。

```cpp
const std::vector<sirkit::KnapsackByValue::Item> items{{3, 4}, {5, 9}, {4, 7}};
const sirkit::KnapsackByValue knapsack(items);
const auto best = knapsack.max_value(8); // 13
const auto weight = knapsack.minimum_weight(11); // 7
```

[使用例](../Examples/knapsack_by_value_example.cpp) ／ [Verify](../Verify/current/submissions/knapsack_by_value_dp_e.cpp) ／ [重さで添字を取るナップサック](knapsack.md)
