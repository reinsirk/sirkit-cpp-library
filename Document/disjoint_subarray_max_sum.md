# 複数の部分配列の最大和

交わらない空でない区間をちょうど `k` 個選び、要素の合計を最大化します。区間 `[0, 2)` と `[2, 3)` のように、端が接する区間も選べます。

| API | 内容 |
| --- | --- |
| `DisjointSubarrayMaxSum(values)` | 数列を前処理する。$O(n\log(n+1))$ 時間・$O(n)$ 領域。 |
| `size()` | 元の数列の長さ。$O(1)$。 |
| `max_sum(k)` | `k` 区間の最大和。$O(1)$。 |
| `intervals(k)` | 最大和を達成する半開区間の一例を左から返す。$O(n)$ 時間・領域。 |

`k = 0` では和が `0` で区間は空です。`k > n` なら `max_sum` と `intervals` は `std::nullopt` を返します。重みと計算途中の和・差は `Score` に収まる必要があります。既定の `Score` は `std::int64_t` です。

```cpp
const std::vector<int> a{4, -5, 6, 2};
const sirkit::DisjointSubarrayMaxSum data(a);
const auto sum = data.max_sum(2).value(); // 12
const auto ranges = data.intervals(2).value(); // [0,1), [2,4)
```

[使用例](../Examples/disjoint_subarray_max_sum_example.cpp) ／ [ローカルVerify](../Verify/current/submissions/disjoint_subarray_max_sum.cpp) ／ [帰着の解説](https://info.atcoder.jp/entry/algorithm_lectures/matching_on_path)
