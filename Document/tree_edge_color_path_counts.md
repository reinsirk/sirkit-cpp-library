# 木のパスに現れる辺色を数える

`tree_edge_color_path_counts(n, edges)` は、辺色がちょうど 1 種類または 2 種類のパスを数えます。同じ 2 頂点の組は向きによらず 1 回です。

辺を `TreeColorEdge{from, to, color}` で渡します。頂点番号は 0 始まりで、色は任意の `int64_t` です。

戻り値の `one_color` と `two_colors` がそれぞれの件数です。重心から各枝へ伸びるパスの色集合を集め、異なる枝を結ぶパスと重心に着くパスを数えます。

計算量は `O(n log²(n+1))` 時間・`O(n)` 領域です。

[使用例](../Examples/tree_edge_color_path_counts_example.cpp) · [Verify問題](https://yukicoder.me/problems/no/1002)
