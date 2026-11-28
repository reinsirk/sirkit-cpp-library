# 集合内を移動できるグラフの最短路

複数の頂点をまとめた集合があり、同じ集合の任意の頂点へ一定の費用で移動できる場合の最短路を求めます。集合は重複してよく、普通の辺も併用できます。

`HypergraphShortestPaths(n, groups)` に `Hyperedge{members, cost}` の配列を渡します。頂点番号は 0 始まりです。集合の移動費用を省略すると 1 になります。

- `unit_distances(source, adjacent)` は全移動の費用が 1 の場合に使います。普通の辺は `adjacent(v, visit)` 内から `visit(to)` で列挙します。未到達の距離は `-1` です。
- `weighted_distances(source, adjacent)` は非負の `uint64_t` 費用を扱います。普通の辺は `visit(to, cost)` で列挙します。未到達や型の範囲を超える距離は `nullopt` です。

いずれも普通の辺がなければ `adjacent` を省略できます。各集合を調べるのは、最初のメンバーの最短距離が確定したときの 1 回です。頂点数を n、普通の辺数を m、集合数を G、全メンバー数を L とすると、時間は費用 1 で `O(n + m + G + L)`、重み付きで `O(n + G + (m + L) log(m + L + 2))` です。

[使用例](../Examples/hypergraph_shortest_paths_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc302/tasks/abc302_f)
