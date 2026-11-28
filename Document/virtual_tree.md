# 圧縮木（Virtual Tree）

元の木から指定頂点とそれらの最小共通祖先を取り出し、間のパスを1辺に縮めます。色ごとの距離和など、少数の頂点に絞った処理を繰り返すときに使います。[本体](../Graph/Virtual_Tree.cpp)

## `VirtualTree`

```text
VirtualTree(int n, std::span<const std::pair<int, int>> edges, int root = 0);
```

頂点番号が $0$ から $n-1$ の無向木を前処理します。辺は端点の組で渡し、入力は保持しません。空の木は辺を空にして既定の根を指定します。通常版では、根や端点が範囲外の場合や入力が木をなさない場合に例外を投げます。非負の頂点数が必要です。

時間・領域はともに $O(n)$ です。

## `build`

```text
std::vector<Node> build(std::span<const int> terminals) const;
```

元の木の `root` を基準に、`terminals` とそれらの最小共通祖先で圧縮木を作ります。頂点の順序は自由で、重複はまとめます。指定が空なら結果も空です。通常版では頂点が範囲外なら `std::out_of_range` を投げます。

- `Node::vertex` は元の頂点番号、`Node::parent` は結果の配列内での親の添字です。親は子より前に並び、根の `parent` は `-1` です。
- `Node::distance` は元の木で親に至る辺の本数です。根では `0` です。
- `Node::terminal` は頂点が入力に含まれていれば `true` です。

指定数を $k$ とすると、時間は $O(k\log(k+1)+k\log(n+1))$、追加領域は $O(k)$ です。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{1, 3}};
const sirkit::VirtualTree tree(4, edges);
const std::array terminals{2, 3};
const auto nodes = tree.build(terminals);  // 根は頂点1。子は頂点2と3。
```

[使用例](../Examples/virtual_tree_example.cpp)・[距離積のVerify](../Verify/current/submissions/virtual_tree_distance_product.cpp)・[同色の距離和のVerify](../Verify/current/submissions/virtual_tree_color_distance.cpp)
