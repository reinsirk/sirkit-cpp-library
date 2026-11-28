# 連続部分列の集約値をまとめて列挙

`for_each_subarray_aggregate_run(input, combine, visit)` は、右端 `r` を一つずつ進め、各区間 `[l,r]` の集約値を調べます。集約値が等しく、左端が連続する区間は一つにまとめます。

- `visit(r, runs)` に届く各ランには集約値 `value` と左端の範囲 `[first,last)` が入ります。`runs` は呼び出し中だけ使えます。
- `combine(a,b)` は左から順に適用する結合的な演算です。`equal` を指定して値をまとめる場合は、同値な値に同じ要素を加えた結果も同値であることが必要です。
- 右端ごとのラン数を $K_r$ として、演算・比較は $O(n+\sum_r K_r)$ 回、作業領域は $O(\max_r K_r)$ です。$w$ ビットの GCD、AND、OR では $K_r=O(w)$ ですが、正整数の和では $r+1$ 個になることがあります。

たとえば `[6,10,15]` の右端が2のとき、GCD は左端0から順に `1,5,15` です。各右端で GCD が1のランの長さを足せば、GCD が1になる連続部分列の個数が求まります。

[使用例](../Examples/subarray_aggregate_runs_example.cpp) ／ [Verify](../Verify/current/submissions/subarray_aggregate_runs_yuki1036.cpp)
