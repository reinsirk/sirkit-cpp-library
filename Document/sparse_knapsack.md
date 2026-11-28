# 疎な0/1ナップサック

重さも価値も大きく、重さや価値を添字にした表が作りにくいときに使えます。各品物を1回まで選び、より軽くて価値が同じか高い候補があれば、重い方を捨てます。

| API | 操作・計算量 |
|---|---|
| `SparseKnapsack(capacity, items)` | `Item{weight, value}` の列を前計算。$n$ 品、処理 $i$ の直前の候補数を $P_i$ として $O(n\log(n+1)+\sum P_i)$ 時間、$O(n+\max P_i)$ 領域。 |
| `max_value(budget)` | 容量 `budget` に収まる最大価値。`budget` は前計算の `capacity` 以下。省略すると `capacity`。最終候補数を $P$ として $O(\log(P+1))$。 |
| `frontier()` | 価値が正の候補を `std::span<const Item>` で参照。重さ・価値とも昇順。$O(1)$。 |

重さ・価値・容量は `std::uint64_t` です。選ばないと価値0になります。前計算の容量に収まる価値が `std::uint64_t` を超えたら `std::overflow_error` です。

```cpp
const std::vector<sirkit::SparseKnapsack::Item> items{{3, 4}, {5, 9}, {4, 7}};
const sirkit::SparseKnapsack knapsack(8, items);
const auto best = knapsack.max_value(); // 13
const auto small = knapsack.max_value(4); // 7
```

[使用例](../Examples/sparse_knapsack_example.cpp) ／ [Verify](../Verify/current/submissions/sparse_knapsack_yuki626.cpp)
