# 昇順を保つ値域加算

昇順の整数列で、現在の値が指定した範囲にある要素をまとめて増減します。更新後も列は昇順です。同じ値が連続しても構いません。

## `MonotoneValueShift`

```cpp
MonotoneValueShift();
explicit MonotoneValueShift(std::span<const std::int64_t> values);
```

空の列、または昇順の列から作ります。長さ $n$ は `INT_MAX` 以下です。$O(n+1)$ 時間・領域を使います。

## `size`

要素数を $O(1)$ 時間で返します。

## `empty`

空かどうかを $O(1)$ 時間で返します。

## `get`

0始まりの位置 `i` にある値を返します。

## `lower_bound`

`value` 以上の値が初めて現れる位置を返します。該当する値がなければ `size()` を返します。

## `upper_bound`

`value` より大きい値が初めて現れる位置を返します。該当する値がなければ `size()` を返します。

## `count_between`

閉区間 `[low, high]` に入る値の個数を返します。

## `shift_between`

更新前の値が閉区間 `[low, high]` に入る全要素に `delta` を加えます。更新後も昇順で、更新した値が `std::int64_t` の範囲に収まることが前提です。

`get` から `shift_between` までは各 $O(1+\log(n+1))$ 時間です。検査が有効な場合、昇順でない入力、範囲の逆転、昇順を崩す更新には `std::invalid_argument`、数値の範囲を超える更新には `std::overflow_error`、範囲外の位置には `std::out_of_range` を送出します。失敗した更新で値は変わりません。

## `values`

現在の列をコピーして返します。$O(n+1)$ 時間・領域を使います。

## 使用例

```cpp
const std::vector<std::int64_t> initial{0, 2, 2, 5};
sirkit::MonotoneValueShift sequence(initial);
sequence.shift_between(0, 2, 2); // {2, 4, 4, 5}
assert(sequence.count_between(4, 5) == 3);
sequence.shift_between(4, 4, -2); // {2, 2, 2, 5}
```

[実装](../DataStructure/Monotone_Value_Shift.cpp)・[使用例の全体](../Examples/monotone_value_shift_example.cpp)・[Verify](../Verify/current/submissions/monotone_value_shift.cpp)
