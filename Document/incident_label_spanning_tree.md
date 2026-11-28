# 各頂点が指定ラベルの辺に接する全域木

`incident_label_spanning_tree<Label>(required, edges)` は、すべての頂点 `v` が `required[v]` と同じラベルの辺に少なくとも 1 本接する全域木を求めます。

入力は無向辺 `LabeledSpanningEdge<Label>{from, to, label}` です。頂点と辺番号は 0 始まりで、ラベルは等値比較できる型を使います。

戻り値は採用した辺の入力番号です。条件を満たす全域木が存在しなければ `std::nullopt`、空グラフなら空列です。

両端を満たす辺から始め、満たした頂点に隣接する頂点を順に加えます。最後に残った成分をつなぎます。計算量は `O(n+m α(n+1))` 償却時間・`O(n+m)` 領域です。

[使用例](../Examples/incident_label_spanning_tree_example.cpp) · [Verify問題](https://atcoder.jp/contests/agc064/tasks/agc064_b)
