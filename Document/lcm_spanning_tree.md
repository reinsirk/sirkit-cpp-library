# 最小公倍数で重みを付けた最小全域木

`minimum_lcm_spanning_tree(labels)` は、各頂点に正の整数を割り当て、任意の2頂点を結ぶ辺の重みをそのラベルの最小公倍数としたときの最小全域木を返します。

- `labels` は `uint32_t` の列です。ラベルは重複でき、頂点数は `INT_MAX` 以下です。頂点番号は0始まりです。
- 返す辺には `from`、`to`、`weight` があり、重みは `uint64_t` です。空の列と頂点1個は空の辺列を返します。ラベル0は `std::invalid_argument` になります。
- 頂点数を $n$、最大ラベルを $U$、最大公約数で割った最大ラベルを $A$ とすると、$O(n\log(U+1)+A\log^2(A+1))$ 時間、$O(n+A\log(A+1))$ 領域です。最小ラベルが全ラベルを割る場合は $O(n\log(U+1))$ 時間です。

たとえばラベルが `{6,9,12,18}` のとき、選んだ辺の重みの和は $48$ です。最小公倍数の計算はライブラリが行うので、入力で辺を列挙する必要はありません。

[使用例](../Examples/lcm_spanning_tree_example.cpp) ／ [Verify](../Verify/current/submissions/lcm_spanning_tree_yuki3827.cpp)
