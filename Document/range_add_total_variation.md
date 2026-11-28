# 区間加算と隣接差の絶対値和

整数列の連続区間に同じ値を足しながら、隣り合う2値の差の絶対値をすべて合計した値を求めます。

## `RangeAddTotalVariation(values)`

`std::int64_t` の列から構築します。列長を $n$ とすると $O(n+1)$ 時間・領域です。

## `range_add(left, right, delta)`

0始まりの半開区間 `[left, right)` に `delta` を足します。変わる隣接差は区間の両端だけなので $O(1)$ です。

## `total_variation()`

隣接差の絶対値和を `std::uint64_t` で返します。構築時と各更新後の合計をこの型に収めてください。$O(1)$ です。

## `size()`

列の長さを $O(1)$ で返します。

## 使用例

```cpp
const std::array<std::int64_t, 3> values{1, 2, 3};
sirkit::RangeAddTotalVariation variation(values);
variation.range_add(1, 3, 1);
assert(variation.total_variation() == 3);
```

[実装](../DataStructure/Range_Add_Total_Variation.cpp)・[使用例の全体](../Examples/range_add_total_variation_example.cpp)・[Verify](../Verify/current/submissions/range_add_total_variation.cpp)
