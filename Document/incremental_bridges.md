# 辺追加に対する橋と 2 辺連結成分

無向グラフに辺を追加しながら、橋と 2 辺連結成分を調べます。取り除くと連結成分が増える辺が橋です。グラフからすべての橋を除くと、2 辺連結成分に分かれます。

`IncrementalBridges(n)` は、0 始まりの頂点を n 個持つ空のグラフを作ります。`add_edge(u, v)` は無向辺の ID を 0 始まりの追加順で返します。多重辺・自己辺も入力可能です。

- `is_bridge(id)` と `bridge_count()` は現在の橋の判定と総数を返します。
- `same_component(u, v)` と `same_two_edge_component(u, v)` は、二頂点が同じ連結成分や 2 辺連結成分にあるかを返します。
- `component_count()` と `block_count()` はそれぞれの成分数を返します。

橋の判定と各総数の取得は `O(1)` です。m 回の辺追加と q 回の成分クエリに `O((m + q + n log(n + 1)) α(n + 1))` 時間、`O(n + m)` 領域を使います。α は逆アッカーマン関数です。

[使用例](../Examples/incremental_bridges_example.cpp) · [Verify](https://yukicoder.me/problems/no/3463) · [橋の静的な列挙](low_link.md)
