# 種類数と出現回数で連続部分列を探す

種類数や各値の出現回数に上限があるとき、空でない連続部分列の個数と最長区間を求めます。

## `SubarrayFrequencyBounds`

`maximum_distinct` は区間内の異なる値の個数、`maximum_occurrences` は区間内で同じ値が現れる回数の上限です。省略した条件は無制限です。

## `SubarrayFrequencyResult`

`count` は条件を満たす区間の個数です。`left`、`right` は最長区間の半開区間 `[left, right)` の端で、0始まりです。同じ長さなら左端が最小の区間を返します。該当する区間がなければ、3つとも0です。

## `length()`

最長区間の長さ `right - left` を $O(1)$ で返します。

## `analyze_frequency_bounded_subarrays(values, bounds)`

入力には `vector<int>`、`array`、`string_view` など、64ビット以下の整数型の要素を持つ前方向範囲を渡せます。同じ値が2回以上現れない区間を探すときは `maximum_occurrences = 1` を指定します。

尺取り法とハッシュ表を使います。列長を $n$、処理中に表へ入る最大の種類数を $d$ とすると、期待 $O(n+1)$ 時間、$O(d+1)$ 領域です。個数は `std::uint64_t` に収めてください。

## 使用例

```cpp
const std::array values{1, 2, 1, 2, 3};
const auto result = sirkit::analyze_frequency_bounded_subarrays(
    values, {.maximum_distinct = 2, .maximum_occurrences = 1});
assert(result.count == 9);
assert(result.left == 0 && result.right == 2);
```

[実装](../Algorithm/Subarray_Frequency_Windows.cpp)・[使用例の全体](../Examples/subarray_frequency_windows_example.cpp)・[種類数のVerify](../Verify/current/submissions/subarray_frequency_windows_distinct.cpp)・[重複を許さないVerify](../Verify/current/submissions/subarray_frequency_windows_unique.cpp)
