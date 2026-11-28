# 辺をちょうど k 本通る移動の集約

同じ辺を繰り返し使える有向グラフで、出発点からちょうど k 歩の移動を到着点ごとに集約します。辺の重みを通った順に `multiply` し、同じ頂点に着く結果を `add` でまとめます。

`fold_exact_length_walks(n, edges, source, k, zero, one, add, multiply)` に `ExactLengthWalkArc<T>{from, to, weight}` を渡します。頂点番号は 0 始まりです。到着できない頂点の値は `zero` で、0 歩の場合は出発点だけに `one` が入ります。

演算と単位元は半環を成し、値は `zero` と比較できる必要があります。たとえば経路数には加算と乗算、重み上限の最小化には `min` と `max` を使います。

k が n 以下なら `O((n+m)(k+1))` 時間・`O(n)` 追加領域です。k が n を超えるときは行列の二乗を使い、`O(m+n³ log(k+1))` 時間・`O(n²)` 追加領域です。

[使用例](../Examples/exact_length_walk_fold_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc236/tasks/abc236_g)
