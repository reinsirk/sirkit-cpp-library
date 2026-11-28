# 辞書順最小の最短路

始点から終点までの最短路が複数あるとき、通る頂点列が辞書順で最小の路を求めます。頂点列も同じなら、辺 ID 列が辞書順で最小の路が選ばれます。

`lexicographic_shortest_path(n, edges, source, target, undirected=false)` に頂点数、辺の列、始点、終点を渡します。辺は `ShortestPathEdge{from, to, weight}` です。頂点は 0 始まりで、重みは正の `uint64_t` です。

返り値には距離 `distance`、頂点列 `vertices`、入力順の辺 ID 列 `edges` が入ります。終点に届かない場合や距離が `uint64_t` を超える場合は `nullopt` です。

終点から逆向きに距離を求め、残りの距離が最短のままになる隣の頂点から最小番号のものを選びます。時間は `O(n + m log(m + 1))`、領域は `O(n + m)` です。

[使用例](../Examples/lexicographic_shortest_path_example.cpp) · [Verify](https://yukicoder.me/problems/no/160)
