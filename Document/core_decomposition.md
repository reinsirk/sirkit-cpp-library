# k-core 分解

`core_decomposition(n, edges)` は 0 始まりの無向グラフを受け取ります。各頂点が属する最大の k-core の `k` を `number[v]` に返します。k-core は残る頂点の次数がすべて `k` 以上になる最大の誘導部分グラフです。例えば、三角形に葉を 1 個つなぐと三角形の頂点が 2、葉が 1 です。

`weighted_core_decomposition<T>(n, edges, weights)` は次数を隣接頂点の重みの和に置き換えます。重みは非負で、総和が型 `T` に収まるようにしてください。多重辺は本数分数えます。

どちらの結果も `order` が頂点の削除順で、`degeneracy` が最大のコア番号です。この順に削除すると、削除時の次数や隣接重みの和の最大値を最小化できます。

通常版は `O(n+m+1)` 時間、重み付きは `O((n+m+1) log(n+m+2))` 時間です。領域はともに `O(n+m+1)` です。

[使用例](../Examples/core_decomposition_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc267/tasks/abc267_e)
