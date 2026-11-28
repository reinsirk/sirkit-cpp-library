# 静的トップ木

頂点の値を更新しながら、固定した根で木DPの答えを求めます。横の枝をまとめる操作（rake）と、上下のパスをつなぐ操作（compress）でDPを書けるときに使えます。

## `StaticTopTree`

```text
template<class Policy>
StaticTopTree(std::span<const Vertex> values, std::span<const Edge> edges,
              int root = 0, Policy policy = {})
```

頂点数 $N$ の空でない無向木と、各頂点の初期値を渡します。頂点番号は0始まりです。`Edge` は両端を表す `std::pair<int, int>` です。辺と根は構築後には変えられません。

入力検査を有効にしている場合、根が範囲外なら `std::out_of_range`、辺の指定が木にならなければ `std::invalid_argument` を送出します。

`Policy` は頂点値の型 `Vertex`、パスの要約の型 `Path`、枝の要約の型 `Point` を定義します。4つの操作は `const noexcept` にします。各操作が $O(1)$ なら構築は $O(N\log(N+1))$ 時間、領域は $O(N)$ です。

- `Path vertex(const Vertex &value, const Point *branches) const noexcept` は、頂点と横の枝をまとめて1頂点分のパスを作ります。横の枝がなければ `branches` は `nullptr` です。ポインターは呼び出し中だけ使えます。
- `Point project(const Path &path) const noexcept` は、完成したパスを親の横の枝に変換します。
- `Point rake(const Point &left, const Point &right) const noexcept` は、同じ親につながる横の枝を結合します。結合則と交換則が必要です。
- `Path compress(const Path &upper, const Path &lower) const noexcept` は、根に近いパス `upper` と遠いパス `lower` をその順でつなぎます。結合則が必要です。

`Vertex` はコピー構築できる型、`Path` と `Point` はムーブ構築できる型にします。`Policy` のムーブ構築とムーブ代入、および `Vertex`、`Path`、`Point` のムーブ代入は例外を送出しないようにします。

## `size`

```text
int size() const noexcept
```

頂点数を $O(1)$ で返します。

## `set`

```text
void set(int v, Vertex value)
```

頂点 `v` の値を置き換えて、DPを更新します。入力検査が有効で `v` が範囲外なら `std::out_of_range` を送出します。各コールバックが $O(1)$ なら、$O(\log(N+1))$ 時間です。

## `all_prod`

```text
const Path &all_prod() const
```

根を含む完成したパスの要約への参照を $O(1)$ で返します。必要に応じて `project` と同じ変換を呼んで答えを求めます。`set` で参照先の値は更新され、木の移動・代入・破棄で参照は無効になります。

## 使用例

次の例では頂点値を足し合わせます。`SumOperations` は `vertex` で頂点値と枝の和を足し、`project` で値をそのまま返し、`rake` と `compress` で2つの和を足します。

```cpp
const std::array values{3, 5, 1};
const std::array edges{std::pair{0, 1}, std::pair{0, 2}};
sirkit::StaticTopTree<SumOperations> tree(values, edges);
tree.set(2, 4);
assert(tree.all_prod() == 12);
```

[使用例](../Examples/static_top_tree_example.cpp) ／ [Verify](../Verify/current/submissions/static_top_tree.cpp)
