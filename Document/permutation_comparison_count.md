# 隣接比較を満たす順列の個数

`count_permutations_by_comparisons<Prime>(comparisons)` は、隣り合う要素の大小を指定して、条件を満たす順列の個数を法 `Prime` で返します。文字列の長さが $n-1$ なら、順列の値は $1$ から $n$ です。

- `<` は前の値が小さい、`>` は前の値が大きい、`?` はどちらでもよい、を表します。
- `Prime` は素数です。既定値は $998244353$。$n<\min(\text{Prime},2^{22})$ が必要です。
- 計算量は最悪 $O(n\log^2(n+1))$ 時間、$O(n)$ 領域。無効な文字は `std::invalid_argument`、長さの上限を超えると `std::length_error` になります。

`"<>"` なら、順列 `1,3,2` と `2,3,1` の2通りです。

[使用例](../Examples/permutation_comparison_count_example.cpp) ／ [Verify](../Verify/current/submissions/permutation_comparison_count_dp_t.cpp)
