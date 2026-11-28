# 異なるグループからの最近距離

複数の始点をグループに分けて、各頂点に近いグループを二つ求めます。あるグループを除いた最短距離が欲しいときは、除外対象と異なる方を距離順に選べば求まります。

`two_nearest_source_groups(n, edges, sources, undirected=false)` に、頂点数、辺 `{from, to, weight}`、始点 `{vertex, group}` を渡します。辺の重みは非負の `uint64_t` です。頂点番号は 0 始まりで、グループ番号は任意の `int` です。有向グラフでは始点から各頂点に向かう距離を求めます。

`result[v][0]` と `result[v][1]` は頂点 `v` に近い順の候補です。各候補に `distance`、始点の頂点番号 `source`、`group` が入ります。距離が等しいときの順番は未規定です。候補が足りない場合は `nullopt` が入ります。

辺数を `m`、始点数を `s` とすると、領域は `O(n + m + s)` です。全辺の重みが 1 なら時間も `O(n + m + s)` で、それ以外は `O(n + (m + s) log(m + s + 1))` です。距離が `uint64_t` を超える候補は除外します。

[使用例](../Examples/two_nearest_source_groups_example.cpp) · [重み付きの Verify](https://atcoder.jp/contests/abc245/tasks/abc245_g) · [重みが 1 の Verify](https://atcoder.jp/contests/abc429/tasks/abc429_e)
