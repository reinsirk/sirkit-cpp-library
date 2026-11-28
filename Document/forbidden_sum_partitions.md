# 禁止された区間和を避ける分割

整数列を空でない連続区間に分け、どの区間の和も禁止値に当たらない分け方を数えます。

## `count_partitions_avoiding_sums<Count>`

```cpp
template <typename Count>
Count count_partitions_avoiding_sums(std::span<const std::int64_t> values,
                                    std::span<const std::int64_t> forbidden);
```

`forbidden` には値の列か `std::int64_t` の単一値を渡せます。禁止値の列は順不同で、重複も構いません。空の入力列の分割数は1です。

`Count{}` で0、`Count{1}` で1を作れ、`+=` と `-=` を使える型を指定します。剰余を求める場合は modint などが使えます。厳密な個数を求める場合は、途中の累計も `Count` に収めてください。

同じ累積和に到達する切れ目の分割数を一つのハッシュ表にまとめます。列長を $n$、指定した禁止値の数を $k$ とすると、演算が $O(1)$ の場合は期待 $O(1+n(k+1)+k\log(k+1))$ 時間、$O(n+k+1)$ 領域です。各累積和は `std::int64_t` に収めてください。検査が有効なら超過時に `std::overflow_error` を送出します。

## 使用例

```cpp
const std::array<std::int64_t, 3> values{1, 2, 3};
const auto count = sirkit::count_partitions_avoiding_sums<std::uint64_t>(values, 3);
assert(count == 2);
```

[実装](../Algorithm/Forbidden_Sum_Partitions.cpp)・[使用例の全体](../Examples/forbidden_sum_partitions_example.cpp)・[Verify](../Verify/current/submissions/forbidden_sum_partitions.cpp)
