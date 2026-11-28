# 完全二分木の番号と距離

頂点を $1$ から $n$ まで番号付けし、頂点 `v > 1` の親が `v / 2` となる木を扱います。頂点や辺を配列に持たないので、`uint64_t` に収まる大きな木でも使えます。[本体](../Graph/Complete_Binary_Tree.cpp)

- `CompleteBinaryTree(n)` で作り、`size()` で頂点数を取得します。
- `depth(v)`・`lca(u, v)`・`distance(u, v)` は深さ・最近共通祖先・辺数で測った距離です。
- `kth_ancestor(v, k)` は `v` から親へ $k$ 回進んだ頂点、`kth_on_path(u, v, k)` は `u` を0番目とした経路上の頂点です。範囲外は `0` です。
- `descendant_range(v, k)` は `v` の $k$ 段下の子孫の番号を両端を含む区間で返します。子孫がなければ `std::nullopt` です。
- `count_at_distance(v, k)` は `v` から距離 $k$ の頂点数を返します。共通祖先の候補ごとに子孫の番号範囲を求めるため、$O(\log(n+1))$ 時間・$O(1)$ 領域です。他の操作は $O(1)$ です。

## 使用例

```cpp
const sirkit::CompleteBinaryTree tree(10);
const auto grandchildren = tree.descendant_range(2, 2);  // [8, 10]
const auto count = tree.count_at_distance(2, 2);         // 3, 8, 9, 10 の4頂点
```

[使用例](../Examples/complete_binary_tree_example.cpp)・[Verify](../Verify/current/submissions/complete_binary_tree.cpp)
