# 区間一次加算・区間最小値

区間内の位置 $i$ に応じて $bi+c$ を加え、その後の区間最小値を求めます。

添字は0始まりです。区間は `[left,right)` とし、`0 <= left <= right <= size()` を満たしてください。

## `RangeLinearAddRangeMin`

```text
explicit RangeLinearAddRangeMin(std::span<const std::int64_t> input = {});
```

初期列を渡します。要素数 $n$ は `INT_MAX` 以下です。 $O(n+1)$ 時間・領域です。

## `range_add`

```text
void range_add(int left, int right, std::int64_t b, std::int64_t c);
```

区間の各位置 $i$ に $bi+c$ を加えます。$i$ は元の配列での位置です。空区間では値は変わりません。操作後のすべての要素が `std::int64_t` に収まる必要があります。積や累積した係数は64ビットを超えても扱えます。 $O(1+\sqrt n)$ です。

## `range_min`

```text
std::optional<std::int64_t> range_min(int left, int right) const;
```

区間の最小値を返します。空区間では `std::nullopt` を返します。 $O((1+\sqrt n)\log(n+2))$ です。

## `size`

```text
int size() const noexcept;
```

要素数を返します。 $O(1)$ です。

## 使用例

```cpp
const std::vector<std::int64_t> values{5, 4, 3, 2};
sirkit::RangeLinearAddRangeMin data(values);
data.range_add(1, 4, 2, -1); // {5, 5, 6, 7}
const auto answer = data.range_min(2, 4); // 6
```

[使用例](../Examples/range_linear_add_range_min_example.cpp) ／ [Verify](../Verify/current/submissions/range_linear_add_range_min.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/range_linear_add_range_min)
