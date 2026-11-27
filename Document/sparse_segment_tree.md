# 疎な Segment Tree

ほとんどの要素が単位元の長い配列で、点代入と半開区間の集約を行います。添字を事前登録する必要はありません。関数合成など、順番によって結果が変わる演算も扱えます。

添字は0始まりで、配列長 `n` と端点は `std::uint64_t` です。`op(left,right)` は左から右へ値を合わせる結合的な演算、`e()` は単位元です。`op` と `S` のコピー・ムーブの構築・代入には `noexcept` が必要です。演算の途中の値も `S` で表せるものを使います。演算・値のコピー・比較は定数時間として計算量を示します。

入力検査が有効な場合、範囲外の位置や区間には `std::out_of_range` を送出します。

## `SparseSegmentTree`

```text
template <class S, auto op, auto e>
class SparseSegmentTree;

explicit SparseSegmentTree(std::uint64_t n = 0);
```

長さ `n` の配列を単位元で初期化します。構築は $O(1)$ 時間・領域です。同時に保存した最大点数を $P$ とすると、更新後の領域は $O(P+1)$ です。削除した点の領域は挿入時に再利用します。点数は $2^{31}$ 以下です。

## コピー・移動

```text
SparseSegmentTree(const SparseSegmentTree &other);
SparseSegmentTree(SparseSegmentTree &&other) noexcept;
SparseSegmentTree &operator=(SparseSegmentTree other) noexcept;
```

コピーは独立で、保存領域の大きさに比例します。移動による構築は $O(1)$ で、移動元は長さ0になります。代入では、これに代入前の領域を破棄する時間が加わります。

## `set`

```text
void set(std::uint64_t position, S value);
```

`position < n` の位置に値を代入します。値型が等値比較できる場合、単位元を代入すると点を削除します。償却 $O(1+\log(n+1))$ です。

## `erase`

```text
bool erase(std::uint64_t position);
```

`position < n` の位置を単位元に戻し、保存していた点を削除できた場合に `true` を返します。値型の等値比較は不要です。$O(1+\log(n+1))$ です。

## `get`

```text
S get(std::uint64_t position) const;
```

`position < n` の位置の値を返します。点を保存していない位置の値は単位元です。$O(1+\log(n+1))$ です。

## `prod`

```text
S prod(std::uint64_t left, std::uint64_t right) const;
```

区間 `[left,right)` の値を左から右へ集約します。`left <= right <= n` を満たしてください。空区間は単位元です。$O(1+\log(n+1))$ です。

## `all_prod`

```text
S all_prod() const noexcept;
```

配列全体の集約を $O(1)$ で返します。

## `size`

```text
std::uint64_t size() const noexcept;
```

配列長を $O(1)$ で返します。

## `stored_points`

```text
std::size_t stored_points() const noexcept;
```

保存中の点数を $O(1)$ で返します。等値比較を持たない値型で単位元を代入した点も、`erase` するまでは含みます。

## 使用例

```cpp
sirkit::SparseSegmentTree<std::int64_t, Add, Zero> data(1'000'000'000'000);
data.set(3, 7);
data.set(1'000'000'000'000 - 1, 2);
const auto sum = data.prod(0, 100); // 7
data.set(3, 0);
```

[長大配列の区間アフィン更新・和](sparse_affine_range_sum.md) は、一様な初期値と区間への更新を扱います。

[使用例](../Examples/sparse_segment_tree_example.cpp) ／ [Verify](../Verify/current/submissions/point_set_range_composite_large_array.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/point_set_range_composite_large_array)
