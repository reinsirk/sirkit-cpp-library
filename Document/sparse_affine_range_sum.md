# 長大配列の区間アフィン更新・和

全要素を同じ値で初期化し、区間ごとに値を変換します。長さや端点が大きくても、領域は更新で分割した箇所に比例します。

添字・端点・長さは符号なし64ビット整数です。0始まりの半開区間 `[left,right)` を使い、`left <= right <= size()` を満たしてください。端点の事前登録は不要です。法 `Mod` は正の符号なし32ビット整数で、既定値は998244353です。

## `SparseAffineRangeSum`

```text
template <std::uint32_t Mod = 998244353>
class SparseAffineRangeSum;

explicit SparseAffineRangeSum(std::uint64_t n = 0, std::int64_t initial = 0);
```

長さ $n$ の配列を作り、全要素を `initial` で初期化します。負の初期値も使えます。 $O(1)$ 時間・領域です。

## `range_affine`

```text
void range_affine(std::uint64_t left, std::uint64_t right, std::int64_t a, std::int64_t b);
```

区間の各値 `x` を `a*x+b` に変換します。負の係数も使えます。 償却 $O(1+\log(n+1))$ 時間です。$U$ 回の更新後の領域は $O(1+U\log(n+1))$ です。

## `range_sum`

```text
std::uint32_t range_sum(std::uint64_t left, std::uint64_t right) const;
```

区間和の非負剰余を返します。空区間の和は0です。 $O(1+\log(n+1))$ 時間で節点を追加しません。

## `all_sum`

```text
std::uint32_t all_sum() const noexcept;
```

全要素の和の非負剰余を返します。 $O(1)$ です。

## `size`

```text
std::uint64_t size() const noexcept;
```

配列長を返します。 $O(1)$ です。

## `stored_nodes`

```text
std::size_t stored_nodes() const noexcept;
```

保存した節点の数を返します。 $O(1)$ です。

## 使用例

```cpp
sirkit::SparseAffineRangeSum<> data(1'000'000'000);
data.range_affine(10, 20, 3, 7);
const auto sum = data.range_sum(0, 15); // 5 × 7 = 35
```

[使用例](../Examples/sparse_affine_range_sum_example.cpp) ／ [Verify](../Verify/current/submissions/range_affine_range_sum_large_array.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/range_affine_range_sum_large_array)
