# 根なし木の同型判定と葉削除

頂点番号を無視して、根の指定のない木の形を比較します。辺は頂点番号 0 始まりの `{u, v}` で、各無向辺を 1 回渡します。

- `unrooted_trees_equal(n, first, second)` は 2 つの `n` 頂点の木が同型かを返します。
- `tree_leaf_deletion_matches(n, tree, smaller)` は `n` 頂点の木から葉を 1 つ消して `n-1` 頂点の木にできるとき、その葉の番号をすべて昇順で返します。該当する葉がなければ空です。

頂点数の上限は `INT_MAX / 4` です。葉削除には 2 頂点以上の木を渡してください。判定は重心を根とする部分木の分類で行います。いずれも `O(n log(n + 1))` 時間と `O(n)` 領域です。

[使用例](../Examples/unrooted_tree_isomorphism_example.cpp) · [葉削除の Verify 問題](https://atcoder.jp/contests/nikkei2019-2-final/tasks/nikkei2019_2_final_d)
