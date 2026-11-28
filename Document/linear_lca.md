# 線形時間前処理の最小共通祖先

`LinearLCA(parents)` は、頂点番号 0 始まりの根付き森を親配列から構築します。根の親は `-1` です。親と子の番号順は問いません。

- `get(u, v)` または `lca(u, v)` は最小共通祖先を返します。
- `depth_of(v)` は根からの深さ、`get_distance(u, v)` はパスの辺数です。

異なる木に属する頂点対の祖先と距離は `-1` です。空の森も構築できます。

オイラーツアーの深さが 1 ずつ増減する性質を使って、最小値クエリの表を作ります。前処理は `O(n+1)` 時間・領域、各クエリは `O(1)` 時間です。

[使用例](../Examples/linear_lca_example.cpp) · [AtCoder Algorithm Lectures](https://info.atcoder.jp/entry/algorithm_lectures/linear_time_lca) · [Verify問題](https://judge.yosupo.jp/problem/lca)
