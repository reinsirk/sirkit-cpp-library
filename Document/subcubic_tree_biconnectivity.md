# 次数 3 以下の木の二重頂点連結化

木に辺を追加し、どの 1 頂点を取り除いても残りが連結になるようにします。辺 `(u, v)` を追加する費用は両端の頂点重みの和です。

`augment_subcubic_tree_biconnectivity(weights, edges)` に非負の `uint64_t` の頂点重みと、0 始まりの無向辺を渡します。頂点数は 3 以上で、もとの木の次数はどの頂点も 3 以下です。

戻り値の `edges` が追加する辺、`cost` が最小費用です。最小費用が `uint64_t` の上限を超える場合も辺は得られ、`cost` は `nullopt` になります。

葉を巡回順に並べて半周先と結びます。葉が奇数なら、重みが小さく接続条件を満たす頂点を 1 回加えます。時間と領域はともに `O(n)` です。

[使用例](../Examples/subcubic_tree_biconnectivity_example.cpp) · [AtCoder 公式解説](https://atcoder.jp/contests/arc160/editorial/6387)
