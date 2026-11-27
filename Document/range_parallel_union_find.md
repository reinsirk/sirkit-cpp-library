# 区間対応付き Union Find

長さが等しい二つの区間を、同じ位置の頂点どうしで結合します。成分の合計などを別に管理するときは、実際に結合した二つの代表をコールバックで受け取れます。

頂点は0始まりとし、$n$ を全頂点数、$q$ を操作数、$K$ を処理した区間の最大長とします。$\alpha$ は逆アッカーマン関数です。

## `RangeParallelUnionFind`

```text
explicit RangeParallelUnionFind(std::size_t n = 0);
```

互いに独立した頂点を $n$ 個作ります。$n$ は `INT_MAX` 以下です。 $O(n+1)$ 時間・領域です。

## `merge_range`

```text
template <class OnMerge>
int merge_range(int a, int b, int length, OnMerge &&on_merge);
int merge_range(int a, int b, int length);
```

位置 `i = 0,...,length-1` ごとに `a+i` と `b+i` を結合し、新規に結合した回数を返します。`0 <= length <= n`、`0 <= a,b <= n-length` を満たしてください。空区間や重なった区間も使えます。

`on_merge(keep, absorbed)` は実際に結合した直後に呼ばれます。`keep` は結合後の代表、`absorbed` はもう一方の結合前の代表です。コールバックから参照操作を呼べます。コールバック内で同じオブジェクトの結合や代入はできません。コールバックが例外を送出しても、そこまでの結合は残ります。長さ $2^h$ 用の領域は初めて必要になった時点で確保します。 コールバックを除く構築・$q$ 操作の合計で $O((1+n\log(K+2)+q)\alpha(n+1))$ 時間・$O(1+n\log(K+2))$ 領域です。コールバックは全操作で高々 $\max(0,n-1)$ 回です。

## `merge`

```text
bool merge(int a, int b);
```

頂点 `a` と `b` を結合します。結合によって成分が減った場合は `true` を返します。 償却 $O(\alpha(n+1))$ です。

## `unite`

```text
void unite(int a, int b);
```

頂点 `a` と `b` を結合します。 償却 $O(\alpha(n+1))$ です。

## `root`

```text
int root(int i);
```

頂点 `i` の成分の代表を返します。 償却 $O(\alpha(n+1))$ です。

## `leader`

```text
int leader(int i);
```

頂点 `i` の成分の代表を返します。`root(i)` と同じです。 償却 $O(\alpha(n+1))$ です。

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

## `size`

```text
int size(int i);
int size() const noexcept;
```

`size(i)` は頂点 `i` の成分の頂点数を償却 $O(\alpha(n+1))$ で返します。`size()` は全頂点数を $O(1)$ で返します。

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
sirkit::RangeParallelUnionFind uf(5);
std::vector<int> sums{2, 3, 5, 7, 11};
uf.merge_range(0, 2, 3, [&](int keep, int absorbed)
{
    sums[keep] += sums[absorbed];
});
assert(uf.issame(0, 4));
assert(sums[uf.root(0)] == 18); // 0, 2, 4 の和
```

単一の頂点や頂点の組に対する操作は [Union Find](union_find.md) と同じです。

[使用例](../Examples/range_parallel_union_find_example.cpp) ／ [Verify](../Verify/current/submissions/range_parallel_unionfind.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/range_parallel_unionfind)
