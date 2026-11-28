# 状態付きグラフの最短路

辺のラベルを読むたびに状態が変わるグラフで、終点へ受理状態で到着する最短歩道を求めます。同じ頂点でも、状態が異なれば別の候補です。

`automaton_shortest_path(n, edges, source, target, state_count, initial, transition, accept, undirected=false)` を呼びます。辺は `AutomatonPathEdge{from, to, label, weight}`、頂点と状態は 0 始まりです。重みは非負の `uint64_t` です。

- `transition(state, label)` は次の状態を返します。移動を禁止する場合は `-1` を返してください。
- `accept(state)` は終点でその状態を受理するとき `true` を返します。

状態遷移と受理判定は、同じ引数に対して常に同じ値を返してください。

結果は距離 `distance`、頂点列 `vertices`、状態列 `states`、入力順の辺 ID 列 `edges` です。受理状態に到達できない場合や距離が `uint64_t` を超える場合は `nullopt` です。

状態数を S、辺数を m とします。コールバックが `O(1)` なら、時間は `O(S(n + m log(Sm + 1)))`、領域は `O(S(n + m))` です。全辺の重みが 1 なら `O(S(n + m))` 時間です。

[使用例](../Examples/automaton_shortest_path_example.cpp) · [長さの余りの Verify](https://atcoder.jp/contests/abc132/tasks/abc132_e) · [向き切り替えの Verify](https://atcoder.jp/contests/abc395/tasks/abc395_e)
