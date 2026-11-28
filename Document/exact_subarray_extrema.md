# 最小値と最大値が指定値に等しい区間

連続部分列の最小値と最大値を指定し、両方の値が一致する区間の個数と最長区間を求めます。

## `ExactSubarrayExtremaResult`

`count` には該当する空でない区間の個数が入ります。`left`、`right` は最長区間を表す0始まりの半開区間 `[left, right)` の端です。最長区間が複数あれば左端が最小のものを返し、なければ3つとも0です。

## `analyze_subarrays_with_extrema(values, minimum, maximum, compare)`

`minimum` と `maximum` は同じ値でも構いません。`compare` は既定で `std::less<>{}` を使います。比較順序で `minimum` が `maximum` より後ろに来ないように指定します。

現在の右端までに最後に現れた両極値と、指定範囲外の最後の位置を覚えます。一回の走査で求まり、列長 $n$ に対して $O(n+1)$ 時間、$O(1)$ 領域です。個数は `std::uint64_t` に収めてください。

## 使用例

```cpp
const std::array values{1, 2, 3, 1};
const auto result = sirkit::analyze_subarrays_with_extrema(values, 1, 3);
assert(result.count == 4);
assert(result.left == 0 && result.right == 4);
```

[実装](../Algorithm/Exact_Subarray_Extrema.cpp)・[使用例の全体](../Examples/exact_subarray_extrema_example.cpp)・[Verify](../Verify/current/submissions/exact_subarray_extrema.cpp)
