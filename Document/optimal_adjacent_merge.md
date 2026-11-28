# 隣接併合の最小費用

重みの列から隣り合う2要素を選び、重みの和を持つ1要素に置き換えます。その和を費用とし、列を1要素にする最小費用と併合順を求めます。

## `optimal_adjacent_merge(weights)`

`weights` は非負の `std::uint64_t` の列です。Garsia–Wachs法で期待 $O(n\log(n+1))$ 時間、$O(n)$ 領域です。$n$ は `int` に収めてください。最小費用が `std::uint64_t` に収まらない場合は `std::overflow_error` です。

結果の `cost` が最小費用で、`merges[i]` が $i$ 回目に併合する左右の要素のIDです。元の要素は左から `0` 〜 `n-1`、$i$ 回目にできる要素は `n+i` です。操作も0から数えます。0要素または1要素のときは費用0で、併合順は空です。

```cpp
const std::vector<std::uint64_t> weights{10, 20, 30, 40};
const auto result = sirkit::optimal_adjacent_merge(weights);
// result.cost == 190
// result.merges == {{0, 1}, {4, 2}, {5, 3}}
```

[使用例](../Examples/optimal_adjacent_merge_example.cpp) ／ [Verify](../Verify/current/submissions/optimal_adjacent_merge_dp_n.cpp) ／ [参考論文](https://doi.org/10.1017/S0956796819000194)
