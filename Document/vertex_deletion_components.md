# 各頂点を削除した場合の連結成分数

`vertex_deletion_components(n, edges)` に 0 始まりの無向辺を渡すと、各頂点について、それだけを削除した場合をまとめて調べられます。

- `original_count` は削除前の連結成分数です。
- `after_deleting[v]` は頂点 `v` を削除した後のグラフ全体の連結成分数です。
- `separated_pairs[v]` は、削除前は同じ連結成分にあり、`v` を削除すると分かれる頂点ペアの個数です。削除した頂点を含むペアは数えません。

例えば 0–1–2 の道と孤立頂点 3 があるとき、1 を除くと全体で 3 成分になり、新しく分かれるペアは (0, 2) の 1 組です。自己辺や多重辺も渡せます。

計算量は `O(n+m+1)` 時間・領域です。

[使用例](../Examples/vertex_deletion_components_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc334/tasks/abc334_g)
