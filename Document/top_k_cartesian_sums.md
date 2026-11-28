# 各組から1個ずつ選ぶ和の上位 K 個

各組から値を1つずつ選び、その和を大きい方または小さい方から `k` 個求めます。同じ和になる選び方もそれぞれ数えます。

## `top_k_cartesian_sums(groups, k, order)`

`groups` は `std::vector<std::int64_t>` の列です。`order` の既定は大きい順の `CartesianSumOrder::largest` です。`CartesianSumOrder::smallest` を指定すると小さい順です。選び方が `k` 通り未満なら、存在する分をすべて返します。

`k=0` または空の組があれば空列です。組が0個で `k>=1` なら `{0}` です。返却する和が `std::int64_t` に収まらない場合は `std::overflow_error` です。

組数を $D$、全要素数を $S$、最大の組の要素数を $M$ として、期待 $O(S+Dk\log(k+1))$ 時間、$O(M+k+1)$ 領域です。

```cpp
const std::vector<std::vector<std::int64_t>> groups{{1, 4}, {2, 5}};
const auto sums = sirkit::top_k_cartesian_sums(groups, 3); // {9, 6, 6}
```

[使用例](../Examples/top_k_cartesian_sums_example.cpp) ／ [Verify](../Verify/current/submissions/top_k_cartesian_sums_abc123_d.cpp)
