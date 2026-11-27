# 個数制限付きナップサック

各品目の重さ・価値・個数上限を指定し、選んだ品目の価値を最大にします。

```cpp
const std::vector<sirkit::KnapsackItem> items{{2, 3, 3}, {5, 8}};
auto best = sirkit::maximize_knapsack(7, items);  // 11
```

## `KnapsackItem`

```text
struct KnapsackItem {
    std::size_t weight;
    std::int64_t value;
    std::size_t count = 1;
};
```

重さ・価値・個数上限です。価値は非負とし、重さ0と個数上限0も指定できます。各品目を1個まで選べる場合は `count` を省略します。途中で作れる選択の価値の総和が符号付き64ビットを超えると `std::overflow_error` です。

## `maximize_knapsack`

```text
std::int64_t maximize_knapsack(std::size_t capacity, std::span<const KnapsackItem> items);
```

合計重量が `capacity` 以下になる選び方のうち、最大の価値を返します。 品目数 $n$、指定重量 $W$ に対して $O((n+1)(W+1))$ 時間・$O(W+1)$ 領域です。

## `maximize_exact_knapsack`

```text
std::optional<std::int64_t> maximize_exact_knapsack(
    std::size_t target, std::span<const KnapsackItem> items);
```

合計重量がちょうど `target` になる選び方のうち、最大の価値を返します。選び方がなければ `std::nullopt` です。 品目数 $n$、指定重量 $W$ に対して $O((n+1)(W+1))$ 時間・$O(W+1)$ 領域です。

[使用例](../Examples/knapsack_example.cpp) ／ [Verify](../Verify/current/submissions/knapsack_01.cpp)
