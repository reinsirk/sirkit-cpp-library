# 森の彩色

`ForestColoring(n, edges)` に 0 始まりの無向辺を渡すと、同じ頂点に接する辺の彩色と、距離が 1 または 2 の頂点が異なる色になる彩色を求められます。孤立頂点や非連結の森も扱えます。

- `edge_coloring()` は入力順に各辺の色を返します。必要な色数は最大次数 `Δ` です。
- `distance_two_coloring()` は頂点番号順に各頂点の色を返します。頂点があれば `Δ+1` 色で彩色でき、頂点がなければ 0 色です。
- `count_distance_two<T>(k)` は区別できる色が `k` 個あるときの頂点彩色を数えます。法付きの型 `T` を渡すと、個数をその法で求められます。

彩色の結果の `color_count` が色数で、`colors` が 0 始まりの色の列です。単純な道では辺を 2 色で交互に塗り、頂点は 3 色を繰り返して塗れます。

構築は `O(n+m+1)` 時間・領域で、森では `m < n` です。彩色と数え上げは 1 回 `O(n+1)` 時間で、数え上げの追加領域は `O(1)` です。

[使用例](../Examples/forest_coloring_example.cpp) · [辺の彩色](https://atcoder.jp/contests/abc146/tasks/abc146_d) · [頂点彩色の個数](https://atcoder.jp/contests/abc133/tasks/abc133_e)
