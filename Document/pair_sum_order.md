# ペアの和の順位

すべてのペアを並べずに、和が指定値以下になる組の個数や、指定順位の和を求めます。

## `PairSumOrder`

```cpp
explicit PairSumOrder(std::span<const std::int64_t> values);
PairSumOrder(std::span<const std::int64_t> left, std::span<const std::int64_t> right);
```

1列では `i < j` の組、2列では両方から1要素ずつ選んだ全組が対象です。同じ値でも添字が違えば別の組として数えます。各列長は `UINT32_MAX` 以下とし、合計要素数を $N$ とすると $O((N+1)\log(N+2))$ 時間・$O(N+1)$ 領域で列を複製して整列します。

## `size`

```cpp
std::uint64_t size() const noexcept;
```

全組数を $O(1)$ 時間で返します。

## `sorted_left`

```cpp
std::span<const std::int64_t> sorted_left() const noexcept;
```

昇順に整列した左の列を $O(1)$ 時間で参照します。参照は元のオブジェクトが存続し、代入やムーブで置き換えられるまで有効です。

## `sorted_right`

```cpp
std::span<const std::int64_t> sorted_right() const noexcept;
```

右の列を同じ条件で参照します。1列で構築した場合は `sorted_left()` と同じ列です。

## `count_below`

```cpp
std::uint64_t count_below(std::int64_t x) const noexcept;
```

和が `x` より小さい組の個数を $O(N+1)$ 時間で返します。和が `std::int64_t` の範囲を超える場合も整数として比較します。

## `count_at_most`

```cpp
std::uint64_t count_at_most(std::int64_t x) const noexcept;
```

和が `x` 以下の組の個数を $O(N+1)$ 時間で返します。

## `count_equal`

```cpp
std::uint64_t count_equal(std::int64_t x) const noexcept;
```

和が `x` と等しい組の個数を $O(N+1)$ 時間で返します。

## `kth_smallest`

```cpp
std::optional<std::int64_t> kth_smallest(std::uint64_t rank) const;
```

小さい方から1始まりで `rank` 番目の和を求めます。順位が範囲外なら `std::nullopt`、その和を `std::int64_t` で表せなければ `std::overflow_error` です。$O(64(N+1))$ 時間・$O(1)$ 追加領域で求まります。

## `kth_largest`

```cpp
std::optional<std::int64_t> kth_largest(std::uint64_t rank) const;
```

大きい方から数える場合はこちらを使います。順位、範囲外の動作、計算量は `kth_smallest` と同じです。

## 使用例

```cpp
const std::array<std::int64_t, 4> values{3, 1, 2, 2};
const sirkit::PairSumOrder order(values);
assert(order.count_equal(4) == 2);
assert(order.kth_smallest(1) == 3);
```

[実装](../Algorithm/Pair_Sum_Order.cpp)・[使用例の全体](../Examples/pair_sum_order_example.cpp)・[個数のVerify](../Verify/current/submissions/pair_sum_order_count.cpp)・[順位のVerify](../Verify/current/submissions/pair_sum_order_handshake.cpp)
