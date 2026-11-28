# 木のパス上のアフィン合成の総和

各頂点の値にパス上のアフィン写像を適用し、指定した頂点へ集めた総和を求めます。辺や頂点の値は後から更新できます。[本体](../Graph/Tree_Path_Affine_Sum.cpp)

## `TreePathAffineSum`

```text
TreePathAffineSum(std::span<const T> values, std::span<const Edge> edges);
```

頂点番号は `values` の添字、辺番号は `edges` の添字です。入力は無向木とし、空の木も扱います。`Affine{mul, add}` は $x\mapsto \mathrm{mul}\,x+\mathrm{add}$ を表します。`Edge{u, v, toward_u, toward_v}` の `toward_u` は $v$ から $u$、`toward_v` は逆向きの写像です。整数型を使う場合は中間結果も型に収めてください。法付き計算には `modint` などを使えます。

頂点数 $n$ に対し前処理は $O(n\log(n+1))$ 時間、領域は $O(n)$ です。コピー先は独立した木を持ち、ムーブ後の元は空になります。通常版では辺の端点や木の形を検査します。

## `size`

```text
int size() const noexcept;
```

頂点数を $O(1)$ で返します。

## `set_vertex`

```text
void set_vertex(int v, const T &value);
```

頂点 `v` の値を置き換えます。$O(\log(n+1))$ 時間です。

## `set_edge`

```text
void set_edge(int id, const Affine &transform);
void set_edge(int id, const Affine &toward_u, const Affine &toward_v);
```

1つの写像を渡すと両方向を同じ写像にします。向きごとに値を変える場合は2つ渡します。$O(\log(n+1))$ 時間です。

## `sum_from`

```text
T sum_from(int root) const;
```

各頂点から `root` へ向かう順に辺の写像を適用し、結果を合計します。`root` 自身の値も含めます。$O(\log(n+1))$ 時間です。

## `all_prod`

```text
T all_prod() const;
```

頂点 `0` への総和を $O(1)$ で返します。空の木では `T{0}` です。

通常版では操作で範囲外の番号を指定すると `std::out_of_range` を投げます。計算量は `T` の加算・乗算・コピーを $O(1)$ とした場合です。

## 使用例

```cpp
using Tree = sirkit::TreePathAffineSum<std::int64_t>;
const std::array<std::int64_t, 2> values{7, 10};
const std::array edges{Tree::Edge{0, 1, {2, 6}, {3, 4}}};
Tree tree(values, edges);
auto result = tree.all_prod();  // 7 + (2*10 + 6) = 33
tree.set_edge(0, {1, 0});
```

[使用例](../Examples/tree_path_affine_sum_example.cpp)・[任意根のVerify](../Verify/current/submissions/tree_path_affine_sum.cpp)・[根固定のVerify](../Verify/current/submissions/tree_path_affine_sum_fixed_root.cpp)
