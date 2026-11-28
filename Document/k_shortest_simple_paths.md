# K 本の最短単純路

始点から終点へ向かう単純路を、距離の短い順に最大 K 本求めます。一つの路の中で同じ頂点を二度通りません。距離が同じでも、使う辺の ID 列が異なれば別の路です。

`k_shortest_simple_paths(n, edges, source, target, k, undirected=false)` を呼びます。辺は `ShortestPathEdge{from, to, weight}` の列です。頂点は 0 始まりで、重みは非負の `uint64_t` です。

各結果には `distance`、頂点列 `vertices`、入力順の辺 ID 列 `edges` が入ります。存在する路が K 本未満なら、その本数だけ返します。始点と終点が同じ場合は長さ 0 の路だけが候補です。同距離の順番は未規定で、`uint64_t` を超える距離は除外します。

Yen 法で、採用済みの路から分岐する最短路を探します。最悪時間は `O(kn(n + m log(m+1) + n log(k+1) + k))`、領域は `O(n + m + kn)` です。頂点数・辺数・K が小さいときに使います。

[使用例](../Examples/k_shortest_simple_paths_example.cpp) · [Verify](https://yukicoder.me/problems/no/1069) · [Yen の論文](https://doi.org/10.1287/mnsc.17.11.712)
