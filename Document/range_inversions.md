# 転倒数と区間転倒数

前にある要素のほうが大きい添字の組を数えます。複数区間はまとめて指定できます。

```cpp
const std::vector values{4, 1, 4, 0};
assert(sirkit::inversion_count(values) == 4);
```

## `inversion_count`

```text
template <class T>
std::uint64_t inversion_count(const std::vector<T> &values);
```

$i<j$ かつ `values[j] < values[i]` を満たす組数を返します。長さ $n$ に対して $O(n\log(n+1))$ 時間・$O(n)$ 領域です。

## `range_inversion_counts`

```text
template <class T>
std::vector<std::uint64_t> range_inversion_counts(
    const std::vector<T> &values, std::span<const std::pair<std::size_t, std::size_t>> ranges);
```

各半開区間 `[l,r)` の転倒数を入力順で返します。空区間の答えは0です。Moのアルゴリズムを使い、区間数を $q$、ブロック幅を $b=\max(1,\lfloor n/\sqrt q\rfloor)$ として、$q>0$ なら $O(n\log(n+1)+q\log(q+1)+(n^2/b+qb)\log(n+1))$ 時間・$O(n+q)$ 領域です。$n$ は32ビット符号なし整数の上限以下です。

[使用例](../Examples/range_inversions_example.cpp) ／ [Verify](../Verify/current/submissions/static_range_inversions_query.cpp)
