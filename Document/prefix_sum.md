# 累積和

`sirkit::PrefixSum<T>` は固定した1次元配列の区間和を繰り返し求めるときに使います。添字は0始まりで、区間の右端は含みません。

```cpp
const std::array<std::int64_t, 4> a{3, -1, 4, 2};
const sirkit::PrefixSum<std::int64_t> sums(a);
assert(sums.prefix(2) == 2); // a[0] + a[1]
assert(sums.sum(1, 4) == 5); // a[1] + a[2] + a[3]
```

## `PrefixSum`

```text
explicit PrefixSum(std::span<const T> values = {});
```

長さ `N` の配列から累積和を作ります。空配列も渡せます。`T{}` は加法の単位元で、累積和と差が `T` に収まることが前提です。時間・領域は `O(N)` です。

## `prefix`

```text
T prefix(std::size_t right) const;
```

先頭から `right` 個の和を返します。`right` は `0` 以上 `size()` 以下で、`prefix(0)` は `T{}` を返します。時間は `O(1)` です。

## `sum`

```text
T sum(std::size_t left, std::size_t right) const;
```

半開区間 `[left, right)` の和を返します。`0 <= left <= right <= size()` を満たす端点を渡します。空区間の和は `T{}` で、時間は `O(1)` です。

## `size`

```text
std::size_t size() const noexcept;
```

元の配列の要素数を返します。時間は `O(1)` です。

`prefix` と `sum` の範囲外指定は `std::out_of_range` になります。`SKIP_RUNTIME_CHECKS` を定義する場合は有効な範囲を渡します。

[使用例](../Examples/prefix_sum_example.cpp) · [Verifyコード](../Verify/current/submissions/prefix_sum.cpp) · [Static Range Sum](https://judge.yosupo.jp/problem/static_range_sum)

長方形や直方体の和には[多次元累積和](prefix_sum_nd.md)を使えます。
