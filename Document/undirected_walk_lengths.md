# 無向グラフのちょうど k 歩の移動

辺をちょうど k 回たどって着けるかを調べます。無向グラフでは通った辺を往復して移動を 2 歩延ばせます。

`UndirectedWalkLengths(n, edges, source)` に 0 始まりの無向辺と出発点を渡します。自己辺と多重辺も使えます。

- `minimum_lengths(v)` は頂点 `v` までの最短歩数を偶数・奇数の順で返します。到達できない側は `-1` です。
- `can_walk_to(v, k)` はちょうど k 歩で着けるかを返します。出発点が孤立している場合は 0 歩だけ可能です。
- `vertices_on_walks(v, k)` はその移動で通れるすべての頂点を番号順に返します。

構築と通過頂点の列挙は `O(n+m)` 時間、最短歩数と到着可否の照会は `O(1)` 時間です。

[使用例](../Examples/undirected_walk_lengths_example.cpp) · [Verify問題](https://yukicoder.me/problems/no/1190)
