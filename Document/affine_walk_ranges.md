# 反転と平行移動を伴う有向グラフの歩道

`affine_walk_ranges(n, edges, source)` は、座標 0 で始点から出発し、各頂点に着いたときの座標の最小値と最大値を求めます。

辺は `AffineWalkEdge{from, to, reflect, shift}` です。通過時に `reflect` が真なら座標を `-x + shift` に、偽なら `x + shift` に変えます。頂点と辺は何度通っても構いません。

結果の `reachable` は到達可否、`unbounded` は一方以上の範囲に限りがないことを表します。到達可能かつ有界な頂点では `minimum` と `maximum` が有効です。

反転した回数の偶奇で頂点を 2 種類に分けると、加算だけの歩道になります。計算量は `O(n+m)` 時間・領域です。`shift` の絶対値は `INT32_MAX` 以下とします。

[使用例](../Examples/affine_walk_ranges_example.cpp) · [Verify問題](https://yukicoder.me/problems/no/3784)
