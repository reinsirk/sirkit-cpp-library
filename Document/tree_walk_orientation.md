# 木の上を歩く向きの最適化

`orient_tree_walks(n, edges, walks)` に 0 始まりの無向木と、歩くパスごとの両端を渡します。同じ木辺を一方向だけ使えば 1、両方向を使えば 2 と数え、その合計が最大になるよう各パスの向きを決めます。

戻り値の `covered_directions` は最大値です。`reversed[i]` が真なら、`walks[i]` の端点を入れ替えて歩きます。例えば同じ道を 2 回歩くなら、一度ずつ反対向きに歩くと全辺を両方向に使えます。

葉を共有するパスを組にして向きをそろえ、残りを縮める方法で解きます。頂点数 `n`、パス数 `m` に対して `O(n+m+1)` 時間・領域です。

[使用例](../Examples/tree_walk_orientation_example.cpp) · [Verify問題](https://atcoder.jp/contests/agc025/tasks/agc025_e)
