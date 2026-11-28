# 辺番号列の部分列を使う最短経路

`SubsequenceShortestPaths<T>(n, edges, schedule, source)` は、辺番号を `schedule` に現れる順にしか使えないときの最短距離を求めます。列の各位置は 1 回使うか飛ばせます。同じ辺番号が複数回現れてもかまいません。

- `edges[i]` は `{from, to, weight}` の有向辺です。頂点番号と辺番号は 0 始まりで、負の重みも扱えます。
- `distance_to(v)` は距離を返します。到達できなければ `std::nullopt` です。`distances()` は全頂点分を返します。
- `path_to(v)` は最短経路の一例を `{occurrence, edge_id}` の列で返します。`occurrence` は `schedule` 内の位置です。

構築は `O(n+m+q)` 時間・`O(n+q)` 領域で、`m` は辺数、`q` は番号列の長さです。距離照会は `O(1)`、長さ `L` の経路復元は `O(L+1)` 時間です。

[使用例](../Examples/subsequence_shortest_paths_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc271/tasks/abc271_e)
