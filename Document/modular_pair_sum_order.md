# ペア和の余りの順位

二列からそれぞれ一要素を選び、和を指定した法で割った余りを扱います。すべての組を列挙せずに、余りの大小や順位を調べられます。

## `ModularPairSumOrder`

```text
ModularPairSumOrder(std::span<const std::uint64_t> left,
                    std::span<const std::uint64_t> right, std::uint64_t modulus)
```

正の法 `modulus` で入力を剰余に直し、各列を整列します。同じ値でも添字が異なれば別に数え、各列は `UINT32_MAX` 要素まで受け取れます。長さをそれぞれ $n,m$ として、構築は $O((n+m+1)\log(n+m+2))$ 時間・$O(n+m+1)$ 領域です。コピーは独立し、移動元には組が残りません。

## `size`

```text
std::uint64_t size() const noexcept
```

組の総数 $nm$ を $O(1)$ 時間で返します。

## `count_at_most`

```text
std::uint64_t count_at_most(std::uint64_t bound) const noexcept
```

和の余りが `bound` 以下の組を $O(n+m+1)$ 時間で数えます。

## `kth_smallest`

```text
std::optional<std::uint64_t> kth_smallest(std::uint64_t rank) const noexcept
```

余りを昇順に並べたときの `rank` 番目を返します。順位は1始まりで、範囲外なら `std::nullopt` です。$O((n+m+1)\log(\text{modulus}+1))$ 時間で、クエリあたりの追加領域は $O(1)$ です。

## 使用例

```cpp
const std::array<std::uint64_t, 2> left{2, 9}, right{1, 6};
const sirkit::ModularPairSumOrder order(left, right, 10); // 余りは 0, 3, 5, 8
assert(order.count_at_most(3) == 2);
assert(order.kth_smallest(3) == 5);
```

[実装](../Algorithm/Modular_Pair_Sum_Order.cpp)・[使用例の全体](../Examples/modular_pair_sum_order_example.cpp)
