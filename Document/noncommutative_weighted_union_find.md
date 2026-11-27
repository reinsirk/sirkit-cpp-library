# 非可換群の重み付き Union Find

頂点ごとの値を $A_u$ とし、$A_v=A_u w$ という制約を管理します。$w$ は行列や置換など、掛ける順番で結果が変わる群の要素でも使えます。

`op(a,b)` は積 $ab$、`e()` は単位元、`inverse(a)` は逆元を返します。$S$ はコピー・ムーブ構築と等値比較ができ、ムーブ代入が例外を送出しない型を使います。頂点番号は `0..n-1` です。群の演算と値のコピーを定数時間と数え、$\alpha$ は逆アッカーマン関数とします。頂点を指定する操作は、いずれも1回で最悪 $O(1+\log(n+1))$ です。

## `NoncommutativeWeightedUnionFind`

```text
template <std::equality_comparable S, auto op, auto e, auto inverse>
class NoncommutativeWeightedUnionFind;

explicit NoncommutativeWeightedUnionFind(std::size_t n = 0);
```

互いに独立した頂点を $n$ 個作ります。$n$ は `INT_MAX` 以下です。 $O(n+1)$ 時間・領域です。

## `unite_checked`

```text
MergeResult unite_checked(int from, int to, const S &relation);
```

$A_{to}=A_{from}\,\mathrm{relation}$ を課します。戻り値 `MergeResult` は、新規結合なら `Merged`、既存の制約と一致するなら `AlreadyConsistent`、矛盾なら `Contradiction` です。矛盾時は既存の制約を保ちます。

代表からの差を $g_{from},g_{to}$ とすると、`from` 側の代表を残す場合は、吸収される代表に $g_{from}\,\mathrm{relation}\,g_{to}^{-1}$ を持たせます。`to` 側の代表を残す場合は、この積の逆元を使います。 償却 $O(\alpha(n+1))$ です。

## `constrain`

```text
bool constrain(int from, int to, const S &relation);
```

制約を課し、新しく結合したか、既に制約を満たしていれば `true` を返します。 償却 $O(\alpha(n+1))$ です。

## `merge`

```text
bool merge(int from, int to, const S &relation);
```

制約を課し、新しく結合した場合だけ `true` を返します。 償却 $O(\alpha(n+1))$ です。

## `unite`

```text
void unite(int from, int to, const S &relation);
```

制約を課します。矛盾時は既存の制約を保ちます。 償却 $O(\alpha(n+1))$ です。

## `diff`

```text
S diff(int from, int to);
```

同じ成分の頂点に対して $A_{from}^{-1}A_{to}$ を返します。異なる成分の頂点を渡すと、入力検査が有効な場合は `std::invalid_argument` を送出します。 償却 $O(\alpha(n+1))$ です。

## `try_diff`

```text
std::optional<S> try_diff(int from, int to);
```

同じ成分なら $A_{from}^{-1}A_{to}$、異なる成分なら `std::nullopt` を返します。 償却 $O(\alpha(n+1))$ です。

## `consistent`

```text
bool consistent(int from, int to, const S &relation);
```

制約を追加せず、矛盾するかを確認します。異なる成分か、既存の制約と一致する場合は `true` を返します。 償却 $O(\alpha(n+1))$ です。

## `root`

```text
int root(int vertex);
```

頂点の成分の代表を返します。 償却 $O(\alpha(n+1))$ です。

## `leader`

```text
int leader(int vertex);
```

頂点の成分の代表を返します。`root(vertex)` と同じです。 償却 $O(\alpha(n+1))$ です。

## `same`

```text
bool same(int a, int b);
```

頂点 `a` と `b` が同じ成分に属するかを返します。 償却 $O(\alpha(n+1))$ です。

## `issame`

```text
bool issame(int a, int b);
```

頂点 `a` と `b` が同じ成分に属するかを返します。`same(a,b)` と同じです。 償却 $O(\alpha(n+1))$ です。

## `weight`

```text
S weight(int vertex);
```

成分の代表から頂点への差を返します。 償却 $O(\alpha(n+1))$ です。

## `root_with_weight`

```text
std::pair<int, S> root_with_weight(int vertex);
```

成分の代表と、その代表から頂点への差を順に返します。 償却 $O(\alpha(n+1))$ です。

## `size`

```text
int size(int vertex);
int size() const noexcept;
```

`size(vertex)` は頂点の成分の頂点数を償却 $O(\alpha(n+1))$ で返します。`size()` は全頂点数を $O(1)$ で返します。

## `empty`

```text
bool empty() const noexcept;
```

全頂点数が0なら `true` を返します。 $O(1)$ です。

## `component_count`

```text
int component_count() const noexcept;
```

現在の成分数を返します。 $O(1)$ です。

## 使用例

```cpp
using UF = sirkit::NoncommutativeWeightedUnionFind<Permutation, compose, identity, inverse>;
UF uf(3);
uf.constrain(0, 1, p);
uf.constrain(2, 1, inverse(q));
const auto forward = uf.diff(0, 2); // p*q
const auto backward = uf.diff(2, 0); // inv(q)*inv(p)
```

[重み付き Union Find](weighted_union_find.md) は加法による差を扱います。Library Checker の $A_u=A_v x$ は、このAPIで `constrain(v,u,x)` と書きます。

[使用例](../Examples/noncommutative_weighted_union_find_example.cpp) ／ [Verify](../Verify/current/submissions/unionfind_with_potential_non_commutative_group.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/unionfind_with_potential_non_commutative_group)
