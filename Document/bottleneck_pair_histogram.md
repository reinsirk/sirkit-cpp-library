# パスのボトルネック値ごとの頂点ペア数

重み付き無向グラフの各頂点ペアについて、パスの最大辺重みをなるべく小さくした値を **minimax**、最小辺重みをなるべく大きくした値を **maximin** と呼びます。木ではそれぞれ、そのパスの最大辺重みと最小辺重みです。

`bottleneck_pair_histogram(n, edges)` に頂点数と `BottleneckEdge{from, to, weight}` の列を渡します。頂点は 0 始まりで、重みは `int64_t` です。同じ成分に属する相異なる頂点の順序なしペアを数えます。

結果は重みの昇順です。入力に現れる各 `weight` について、その値になるペア数が `minimax_pairs` と `maximin_pairs` に入ります。該当するペアがなければ 0 です。

辺数を m とすると `O(n + m log(m + 1))` 時間、`O(n + m)` 領域です。

[使用例](../Examples/bottleneck_pair_histogram_example.cpp) · [Verify](https://atcoder.jp/contests/abc214/tasks/abc214_d)
