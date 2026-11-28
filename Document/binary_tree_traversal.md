# 巡回順から二分木を復元

二分木を行きがけ順と通りがけ順から復元します。行きがけ順の代わりに帰りがけ順も使えます。

- `binary_tree_from_preorder_inorder(preorder, inorder)` は行きがけ順から復元します。
- `binary_tree_from_postorder_inorder(postorder, inorder)` は帰りがけ順から復元します。

各列は `0..n-1` の順列です。戻り値の `root` は根、`children[v]` は左の子と右の子です。空木の根や存在しない子は `-1`、復元できない場合は `nullopt` になります。

通りがけ順で根の位置を探し、その位置を境に左右の部分木を分けます。時間と領域はともに `O(n)` です。

[使用例](../Examples/binary_tree_traversal_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc255/tasks/abc255_f)
