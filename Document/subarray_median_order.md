# 連続部分列の中央値の順位

非空の連続部分列すべてについて、中央値が条件を満たす個数や、指定順位の中央値を求めます。同じ値の中央値も、切り出した位置が違えば別々に数えます。

## `SubarrayMedianSide`

`upper` は偶数長で中央2要素の大きい方、`lower` は小さい方を使います。既定値は `upper` です。

## `count_subarray_medians_at_least`

```cpp
std::uint64_t count_subarray_medians_at_least(
    const Sequence &values, const T &threshold, SubarrayMedianSide side = SubarrayMedianSide::upper);
```

中央値が `threshold` 以上の連続部分列の個数を返します。長さを $n$ とすると $O(n+1)$ 時間・領域です。

各要素をしきい値以上なら $+1$、未満なら $-1$ に置き換えます。数えるのは、上側中央値なら和が0以上、下側なら和が正の区間です。累積和は1ずつ増減するので、条件を満たす過去の累積和の個数を定数時間で更新できます。

## `kth_subarray_median`

```cpp
std::optional<T> kth_subarray_median(
    const Sequence &values, std::uint64_t rank, SubarrayMedianSide side = SubarrayMedianSide::upper);
```

中央値の小さい方から1始まりで `rank` 番目の値を返します。順位が範囲外なら `std::nullopt` です。$O((n+1)\log(n+2))$ 時間・$O(n+1)$ 領域です。

どちらの関数でも、`values` は複数回走査でき、長さを定数時間で取得できる範囲です。$n\leq 2^{32}-1$ とします。型 `T` は順序比較でき、計算量は比較・コピーに定数時間かかる場合です。

## 使用例

```cpp
const std::array values{10, 30, 20};
assert(sirkit::count_subarray_medians_at_least(values, 30) == 3);
assert(sirkit::kth_subarray_median(values, 4) == 30);
```

[実装](../Algorithm/Subarray_Median_Order.cpp)・[使用例の全体](../Examples/subarray_median_order_example.cpp)・[Verify](../Verify/current/submissions/subarray_median_order.cpp)
