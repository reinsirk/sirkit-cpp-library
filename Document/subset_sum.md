# 部分和の復元

非負整数列から、指定した合計を作る要素の添字を復元します。

```cpp
const std::vector<std::size_t> values{2, 4, 9};
auto chosen = sirkit::subset_sum(values, 11);
```

## `subset_sum`

```text
std::optional<std::vector<std::size_t>> subset_sum(
    std::span<const std::size_t> values, std::size_t target);
```

合計が `target` になる添字を昇順で返します。同じ添字を2度選びません。解がなければ `std::nullopt`、目標0なら空の列です。目標を $S$ として $O(n(1+S/64)+S)$ 時間・$O(S+1)$ 領域です。

[使用例](../Examples/subset_sum_example.cpp) ／ [Verify](../Verify/current/submissions/subset_sum_cards.cpp)
