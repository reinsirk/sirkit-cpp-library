# 超増加重みの最短路と辺の除外

`SuperincreasingPathQueries<T>(n, edges)` は、無向辺 `i` の長さがそれ以前の全辺の長さの合計より大きいグラフを扱います。たとえば長さが入力順に `1, 2, 4, 8, ...` となる場合です。

辺は `SuperincreasingPathEdge<T>{from, to, value}` で、頂点・辺番号は 0 始まりです。経路選択には入力順を使い、選んだ辺の `value` を集計します。長さの和を法 `P` で求めるなら、各辺の長さを `P` で割った余りを渡せます。

- `path_sum(u, v)` は最短路上の集計値です。
- `path_sum_without_edge(u, v, id)` は指定辺を除外したときの集計値です。ほかのクエリには影響しません。

到達不能なら `std::nullopt`。最小全域森のパスが最短路となり、削除で切れたときは最小の代替辺でつなぎます。構築は `O(n log(n+1) + m(log(m+1)+log(n+1)))` 時間・`O(n log(n+1)+m)` 領域、各クエリは `O(log(n+1))` 時間です。

[使用例](../Examples/superincreasing_path_queries_example.cpp) · [Verify問題](https://yukicoder.me/problems/no/1600)
