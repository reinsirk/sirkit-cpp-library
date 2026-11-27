# 重み付き Wavelet Matrix

固定の配列で、位置の区間とキーの範囲を指定して要素の個数と重み和を求めます。同じキーに異なる重みを付けられます。

添字は0始まりです。照会は半開区間 `[left,right)` を使い、`0 <= left <= right <= size()` を満たしてください。`CountSum` は `std::pair<std::size_t, Sum>` で、個数と重み和の順で返します。

`Sum{}` は加法の零元とし、加減算を持つ可換加法群を使います。`bool` 以外の組み込み整数では型のビット幅で剰余計算し、実際の和が型の範囲内なら厳密に返します。キーの比較と加減算は定数時間とし、相異なるキー数を $\sigma$、$B=\lceil\log_2\sigma\rceil$ とします。空列では $B=0$ とします。

## `WeightedWaveletMatrix`

```text
template <std::totally_ordered Key = std::int64_t, class Sum = std::int64_t>
    requires std::copyable<Key> && std::copyable<Sum> && std::default_initializable<Sum>
class WeightedWaveletMatrix;

WeightedWaveletMatrix() = default;
WeightedWaveletMatrix(std::span<const Key> keys, std::span<const Sum> weights);
```

空、または同じ長さのキー列と重み列から構築します。キーと重みは同じ添字で対応し、列の長さ $n$ は `INT_MAX` 以下です。 $O(1+n\log(n+1))$ 時間・$O(1+n(1+B))$ 領域です。

## `sum`

```text
Sum sum(std::size_t left, std::size_t right) const;
```

位置の区間全体の重み和を返します。 $O(1)$ です。

## `count_sum_less`

```text
CountSum count_sum_less(std::size_t left, std::size_t right, const Key &upper) const;
```

キーが `upper` 未満の要素の個数と重み和を返します。上限にはキー型の最大値も指定できます。 $O(1+B)$ です。

## `count_sum_less_equal`

```text
CountSum count_sum_less_equal(std::size_t left, std::size_t right, const Key &upper) const;
```

キーが `upper` 以下の要素の個数と重み和を返します。上限にはキー型の最大値も指定できます。 $O(1+B)$ です。

## `count_sum`

```text
CountSum count_sum(std::size_t left, std::size_t right,
                   const Key &lower, const Key &upper) const;
```

キーが `[lower,upper)` に入る要素の個数と重み和を返します。添字区間が空、または `lower >= upper` なら `{0, Sum{}}` を返します。 $O(1+B)$ です。

## `size`

```text
std::size_t size() const noexcept;
```

要素数を返します。 $O(1)$ です。

## `empty`

```text
bool empty() const noexcept;
```

要素がなければ `true` を返します。 $O(1)$ です。

## 使用例

```cpp
const std::vector<int> keys{3, 1, 3, -2};
const std::vector<std::int64_t> weights{8, 10, -4, 7};
const sirkit::WeightedWaveletMatrix<int> matrix(keys, weights);
const auto [count, total] = matrix.count_sum_less_equal(0, 4, 1); // 2, 17
```

[通常の Wavelet Matrix](wavelet_matrix.md) は、値の取得、個数、順位の照会に使えます。

[使用例](../Examples/weighted_wavelet_matrix_example.cpp) ／ [Verify](../Verify/current/submissions/static_range_sum_with_upper_bound.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/static_range_sum_with_upper_bound)
