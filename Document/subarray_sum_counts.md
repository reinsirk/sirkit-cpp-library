# 和と長さで連続部分列を数える

負の値も含む整数列から、和と長さが指定の範囲に入る空でない連続部分列を数えます。

## `SubarraySumBounds`

`minimum_sum`、`maximum_sum` は和の下限と上限です。端の値も条件に含まれます。省略した端は無制限です。同じ値を両方に指定すると、その値と等しい和を数えます。

`minimum_length`、`maximum_length` は長さの下限と上限です。両端を含み、最小の長さは1以上です。既定では長さ1以上をすべて調べます。

## `count_subarrays_by_sum(values, bounds)`

全要素が非負なら尺取り法で $O(n+1)$ 時間、負の要素を含む場合は累積和と Fenwick 木で $O((n+1)\log(n+2))$ 時間です。領域はどちらも $O(n+1)$ です。

列長は `INT_MAX` 未満とし、先頭からの各累積和は `std::int64_t` に収めてください。和と長さで、下限が上限を超える指定は無効です。

## 使用例

```cpp
const std::array<std::int64_t, 4> values{3, -1, 2, 4};
const auto count = sirkit::count_subarrays_by_sum(
    values, {.minimum_sum = 3, .maximum_sum = 5, .maximum_length = 3});
assert(count == 4);
```

[実装](../Algorithm/Subarray_Sum_Counts.cpp)・[使用例の全体](../Examples/subarray_sum_counts_example.cpp)・[長さ制約のVerify](../Verify/current/submissions/subarray_sum_counts_length.cpp)・[和の等値条件のVerify](../Verify/current/submissions/subarray_sum_counts_equal.cpp)
