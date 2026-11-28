# ペアの積の順位

二つの整数を選んでできる積を、すべて列挙せずに調べます。負の整数・0・重複した値も使えます。

## `PairProductOrder`

```text
explicit PairProductOrder(std::span<const std::int64_t> values)
PairProductOrder(std::span<const std::int64_t> left, std::span<const std::int64_t> right)
```

1列を渡すと異なる添字 `i < j` の組、2列を渡すと各列から一つずつ取るすべての組が対象です。値が同じでも添字が違えば別の組として数えます。各列の長さは `UINT32_MAX` 以下です。

入力の要素数を合計 $n$ として、構築は $O((n+1)\log(n+2))$ 時間・$O(n+1)$ 領域です。コピーは独立し、移動元には組が残りません。

## `size`

```text
std::uint64_t size() const noexcept
```

組の総数を $O(1)$ 時間で返します。

## `count_at_most`

```text
std::uint64_t count_at_most(std::int64_t bound) const noexcept
```

積が `bound` 以下の組数を $O(n+1)$ 時間で数えます。積が `int64_t` の範囲より小さい組も数に含まれます。

## `kth_smallest`

```text
std::optional<std::int64_t> kth_smallest(std::uint64_t rank) const
```

積を昇順に並べたときの `rank` 番目を返します。順位は1始まりで、範囲外なら `std::nullopt` です。順位は有効でも積が `int64_t` に収まらなければ `std::overflow_error` が発生します。二分探索で数え上げを最大64回行い、各クエリの追加領域は $O(1)$ です。

## 使用例

```cpp
const std::array<std::int64_t, 4> values{3, 3, -4, -2};
const sirkit::PairProductOrder products(values);
assert(products.kth_smallest(3) == -6);
assert(products.count_at_most(-6) == 4);
```

[実装](../Algorithm/Pair_Product_Order.cpp)・[使用例の全体](../Examples/pair_product_order_example.cpp)
