# Euler Tour Tree

辺を追加・削除できる森で、頂点、連結成分、親辺で指定した部分木を更新します。部分木は辺 `(v,p)` を取り除いたときの `v` 側です。

```cpp
sirkit::EulerTourTree<> forest(3);
forest.link(0, 1);
forest.link(1, 2);
forest.subtree_add(1, 0, 4);
auto from_one = forest.subtree_sum(1, 0); // 8
auto from_zero = forest.subtree_sum(0, 1); // 0
forest.cut(1, 2);
```

頂点番号は0始まり。型 `T` は既定で `std::int64_t` です。`T{}` を加法の単位元とし、可換な加算と `T(頂点数)` との乗算を使います。途中値も `T` で表せることが前提です。操作中の `T{}` と `T(頂点数)` の構築、演算、代入は例外を送出しないものを使います。

## `EulerTourTree`

```text
explicit EulerTourTree(int n, std::span<const T> initial = {});
EulerTourTree(const EulerTourTree &other);
EulerTourTree(EulerTourTree &&other);
```

頂点だけの森を $O(n+1)$ 時間・領域で作ります。初期値は頂点順で、省略または空の列なら0です。$n=0$ も扱えます。容量は $3n-2$ が `int` に収まる範囲。コピーは独立で $O(n+1)$、移動は $O(1)$ で移動元は空になります。

## `operator=`

```text
EulerTourTree &operator=(EulerTourTree other) noexcept;
```

引数で置き換えます。コピーまたは移動で引数を構築する時間に加え、代入前の森の破棄に $O(n+1)$ 時間がかかります。

## `swap`

```text
void swap(EulerTourTree &other) noexcept;
```

2つの森を $O(1)$ で交換します。

## `size`

```text
int size() const noexcept;
```

頂点数を $O(1)$ で返します。

## `component_count`

```text
int component_count() const noexcept;
```

連結成分数を $O(1)$ で返します。

## `connected`

```text
bool connected(int u, int v);
```

2頂点が同じ成分なら `true`。償却 $O(\log(n+1))$ です。

## `link`

```text
bool link(int u, int v);
```

別成分を1辺で結べば `true`、同じ成分なら `false`。償却 $O(\log(n+1))$ です。

## `cut`

```text
bool cut(int u, int v);
```

辺を削除できれば `true`、辺がなければ `false`。償却 $O(\log(n+1))$ です。

## `get`

```text
T get(int v);
```

頂点の値を返します。償却 $O(\log(n+1))$ です。

## `add`

```text
void add(int v, const T &delta);
```

頂点の値に加算します。償却 $O(\log(n+1))$ です。

## `component_sum`

```text
T component_sum(int v);
```

連結成分の和を返します。償却 $O(\log(n+1))$ です。

## `component_add`

```text
void component_add(int v, const T &delta);
```

連結成分の全頂点に加算します。償却 $O(\log(n+1))$ です。

## `subtree_sum`

```text
std::optional<T> subtree_sum(int v, int parent);
```

辺 `(v,parent)` から見た `v` 側の和を返します。辺がなければ `std::nullopt`。償却 $O(\log(n+1))$ です。

## `subtree_add`

```text
bool subtree_add(int v, int parent, const T &delta);
```

辺 `(v,parent)` から見た `v` 側の全頂点に加算します。辺がなければ `false`、成功時は `true`。償却 $O(\log(n+1))$ です。

[使用例](../Examples/euler_tour_tree_example.cpp) ／ [点加算のVerify](../Verify/current/submissions/dynamic_tree_vertex_add_subtree_sum.cpp) ／ [部分木加算のVerify](../Verify/current/submissions/dynamic_tree_subtree_add_subtree_sum.cpp)
