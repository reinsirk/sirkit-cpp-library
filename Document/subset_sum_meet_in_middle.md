# 部分集合和の半分全列挙

少数の重みから、上限を超えない最も大きな部分集合和や、指定した和を作る選び方の数を求めます。重みの絶対値が大きい場合にも使えます。

## `MeetInMiddleSubsetSum`

```text
explicit MeetInMiddleSubsetSum(std::span<const std::int64_t> weights)
```

44個以下の整数を前処理します。正の重みの総和と負の重みの総和が、それぞれ `std::int64_t` に収まることが前提です。空集合を含め、重みが同じでも元の添字が異なれば別の選び方として数えます。

要素数を $n$ とすると、前処理は $O(2^{\lceil n/2\rceil})$ 時間・領域です。コピー構築は前処理の結果を共有するので $O(1)$ 時間です。

## 照会

```text
std::optional<Selection> select_max_at_most(std::int64_t limit) const
std::uint64_t count_at_most(std::int64_t limit) const
std::uint64_t count_exact(std::int64_t target) const
```

- `select_max_at_most` は `limit` 以下で最大となる和を求めます。該当する部分集合があれば和を `sum` に、選んだ添字を `mask` に入れて返します。元の重みの添字が `i` ならビット `i` が立ちます。該当しなければ `std::nullopt` を返します。
- `count_at_most` は和が `limit` 以下になる部分集合の数を返します。`count_exact` は和が `target` になる部分集合の数を返します。

各照会は $O(2^{\lceil n/2\rceil})$ 時間・$O(1)$ 追加領域です。

## 使用例

```cpp
const std::array<std::int64_t, 4> weights{5, -2, 5, 3};
const sirkit::MeetInMiddleSubsetSum sums(weights);
const auto best = sums.select_max_at_most(4);
assert(best && best->sum == 3);
assert(sums.count_exact(3) == 3);
```

[実装](../Algorithm/Subset_Sum_Meet_In_Middle.cpp)・[使用例の全体](../Examples/subset_sum_meet_in_middle_example.cpp)
