# 木の距離区間と頂点の更新

頂点 `v` からの距離が `[left, right)` に入る頂点を扱います。点加算と区間和、または区間加算と点取得を選べます。

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
const sirkit::TreeContourIndex index(3, edges);
sirkit::VertexAddContourSum<> sums(index);
sums.add(2, 7);
auto value = sums.sum(0, 1, 3);  // 7

sirkit::ContourAddVertexGet<> values(index);
values.add_range(0, 1, 3, 7);
auto point = values.get(2);  // 7
```

頂点番号は0始まりです。距離は `std::int64_t` で、負の端点も使えます。空区間の和は0、加算は何もしません。距離が `INT64_MAX` 以上の頂点は、表せるどの半開区間にも入りません。値の型 `T` の既定値は `std::int64_t`。加法が可換で、`T{}` が単位元、`+=` と `-=` を使います。加減算は例外を送出せず、途中の値も `T` で表せることが前提です。

## `TreeContourIndex`

```text
TreeContourIndex(int n, std::span<const std::pair<int, int>> edges);
```

辺の長さをすべて1として無向木を前処理します。頂点数は0以上で、$2n-1$ が `int` に収まる範囲。$O(1+n\log^2(n+1))$ 時間、$O(1+n\log(n+1))$ 領域です。コピー間で前処理結果を共有します。

## `TreeContourIndex::weighted`

```text
static TreeContourIndex weighted(int n, std::span<const TreeContourEdge> edges);
```

`TreeContourEdge{from, to, length}` を渡します。端点は `int`、長さは非負の `std::int64_t` です。計算量と所有は無重み版と同じです。

## `TreeContourIndex::size`

```text
int size() const noexcept;
```

頂点数を $O(1)$ で返します。

## `VertexAddContourSum`

```text
explicit VertexAddContourSum(TreeContourIndex index, std::span<const T> initial = {});
```

各頂点の初期値を番号順に渡します。省略または空の列ならすべて0です。$O(1+n\log(n+1))$ 時間・領域で、更新状態は独立に持ちます。

## `VertexAddContourSum::add`

```text
void add(int v, const T &delta);
```

頂点 `v` に `delta` を加えます。$O(\log^2(n+1))$ 時間です。

## `VertexAddContourSum::sum`

```text
T sum(int v, std::int64_t left, std::int64_t right) const;
```

距離区間の頂点値の総和を返します。$O(\log^2(n+1))$ 時間です。

## `ContourAddVertexGet`

```text
explicit ContourAddVertexGet(TreeContourIndex index, std::span<const T> initial = {});
```

各頂点の初期値を番号順に渡します。省略または空の列ならすべて0です。$O(1+n\log(n+1))$ 時間・領域で、更新状態は独立に持ちます。

## `ContourAddVertexGet::add_range`

```text
void add_range(int v, std::int64_t left, std::int64_t right, const T &delta);
```

距離区間に含まれる各頂点に `delta` を加えます。$O(\log^2(n+1))$ 時間です。

## `ContourAddVertexGet::get`

```text
T get(int v) const;
```

頂点 `v` の現在値を返します。$O(\log^2(n+1))$ 時間です。

[使用例](../Examples/tree_contour_example.cpp) ／ [区間和のVerify](../Verify/current/submissions/vertex_add_range_contour_sum_on_tree.cpp) ／ [区間加算のVerify](../Verify/current/submissions/vertex_get_range_contour_add_on_tree.cpp)
