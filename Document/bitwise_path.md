# 辺ラベルの OR 最小・AND 最大パス

`minimum_or_path(n, edges, source, target, directed=true)` はパス上の辺ラベルの OR を最小化します。同じ引数の `maximum_and_path` は AND を最大化します。

`BitwisePathEdge{from, to, label}` の頂点番号は 0 始まり、ラベルは `uint64_t` です。無向辺は `directed=false` を指定します。

戻り値は集約値 `value` と、パス順の `vertices`、入力で付けた辺番号 `edge_ids` です。到達不能なら `nullopt` になります。始点と終点が同じときは、辺のないパスと演算の単位元を返します。

上位ビットから順に、条件に合う辺だけで到達できるかを確かめます。調べるビット数を B として `O((B+1)(n+m))` 時間・`O(n+m)` 領域です。

[使用例](../Examples/bitwise_path_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc408/tasks/abc408_e)
