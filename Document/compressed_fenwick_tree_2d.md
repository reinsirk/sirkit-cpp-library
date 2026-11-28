# CompressedFenwickTree2D

座標を先に登録し、各点への加算と長方形内の重みの和を交互に処理します。座標が離れていても、領域の広さに比例する配列は不要です。

## 使い方

```cpp
using Tree = sirkit::CompressedFenwickTree2D<>;
std::vector<Tree::Point> coordinates{{-10, 5}, {2, 1}, {2, 9}};
Tree tree(coordinates);
tree.add({-10, 5}, 4);
tree.add({2, 1}, 7);
tree.add({2, 9}, -3);
assert(tree.sum(-10, 1, 3, 9) == 11);
```

[使用例](../Examples/compressed_fenwick_tree_2d_example.cpp)

長方形は `[l,r) × [b,t)` です。加算する座標は構築時に登録し、同じ座標への加算は一つの点に合算します。未登録の座標を `add(point,delta)` に渡すと `false` を返します。点のIDを使う場合は `point_id(point)` で取得します。重みは既定で符号付き64ビット整数です。

## 計算量

登録する座標が `n` 個のとき、構築と領域は `O(n log(n+1)+1)`、加算と長方形和は `O(log²(n+1)+1)` です。
