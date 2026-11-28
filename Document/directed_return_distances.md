# 各頂点から出発して戻る最短距離

`directed_return_distances(n, edges)` は、有向グラフの各頂点から 1 本以上の辺を通って出発点に戻る最短距離を返します。

辺は `ReturnDistanceArc{from, to, weight}` で、頂点番号は 0 始まり、重みは `uint64_t` です。自己辺・多重辺・重み 0 の辺も扱えます。

戻れない頂点や最短距離が `uint64_t` の範囲を超える頂点の答えは `nullopt` です。

強連結成分内だけを探索します。成分 c の頂点数と内部辺数を `n_c, m_c` とすると、最悪の時間は `O(n+m+Σ n_c m_c log(m_c+1))`、領域は `O(n+m)` です。成分が 1 本の閉路なら辺の和で求めます。

[使用例](../Examples/directed_return_distances_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc191/tasks/abc191_e)
