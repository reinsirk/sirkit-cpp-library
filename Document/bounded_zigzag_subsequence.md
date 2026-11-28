# 差の範囲を指定した交互増減部分列

隣り合う値の差に上下限があり、増加と減少が交互になる部分列を選びます。各要素に重みがあり、選んだ要素の重みの和を最大にします。

## `bounded_zigzag_subsequence`

```cpp
BoundedZigzagSelection bounded_zigzag_subsequence(
    std::span<const std::int64_t> values, std::span<const std::int64_t> weights,
    std::uint64_t min_step, std::uint64_t max_step);
```

値列と重み列から最適な選び方を1つ返します。隣り合う選択値の差の絶対値は閉区間 `[min_step, max_step]` に含まれ、最初は増加・減少のどちらでもかまいません。空の選択の重みは0です。

長さ $n$ に対し $O(n\log(n+1)+1)$ 時間・$O(n+1)$ 領域です。2列は同じ長さで `INT_MAX` 以下、`1 <= min_step <= max_step` が必要です。長さの不一致や不正な差の範囲は `std::invalid_argument`、最大重み和が `std::int64_t` を超える場合は `std::overflow_error` になります。

## `BoundedZigzagSelection`

`weight` は最大重み和、`indices` は選んだ位置を0始まりの昇順で並べた列です。空の選択が最適なら、`indices` は空です。

## 使用例

```cpp
const std::array<std::int64_t, 3> heights{1, 3, 2};
const std::array<std::int64_t, 3> weights{5, 4, 6};
const auto answer = sirkit::bounded_zigzag_subsequence(heights, weights, 1, 3);
assert(answer.weight == 15);
```

[実装](../Algorithm/Bounded_Zigzag_Subsequence.cpp)・[使用例の全体](../Examples/bounded_zigzag_subsequence_example.cpp)・[Verify](../Verify/current/submissions/bounded_zigzag_subsequence.cpp)
