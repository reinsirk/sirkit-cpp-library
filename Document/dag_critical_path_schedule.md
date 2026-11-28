# DAG のクリティカルパスと作業の余裕

作業の順序と所要時間から、プロジェクトの最短完了時刻を求めます。余裕が 0 の頂点や辺は、いずれかのクリティカルパス上にあります。

`dag_critical_path_schedule(n, edges, start, finish)` に頂点数、`DagScheduleEdge{from, to, duration}` の列、開始・完了の頂点を渡します。頂点は 0 始まりで、所要時間は `uint64_t` です。閉路がある場合や完了に届く経路がない場合は `nullopt` です。

- `duration` は最短完了時刻です。
- `earliest[v]` と `latest[v]` は頂点の最早・最遅時刻で、差が頂点の余裕です。
- `edge_slack[i]` は入力順の辺 ID `i` の余裕です。始点から終点へ向かう経路に含まれない頂点や辺の時刻・余裕は `nullopt` になります。
- `critical_path` はクリティカルパス一つを表す辺 ID 列です。

時間・領域はともに `O(n + m)` です。完了時刻が `uint64_t` を超える場合は `overflow_error` を送出します。

[使用例](../Examples/dag_critical_path_schedule_example.cpp) · [Verify](https://yukicoder.me/problems/no/468)
