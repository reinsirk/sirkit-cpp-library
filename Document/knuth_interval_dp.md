# Knuth最適化による区間DP

区間を二分する最小費用のDPを計算します。隣接した区間を順に併合する問題などで、費用が単調性と四辺形不等式を満たす場合に使えます。

## `KnuthIntervalDP`

```cpp
KnuthIntervalDP(int size, Cost cost);
KnuthIntervalDP(int size, int max_length, Cost cost);
```

半開区間 `[l, r)` の長さが0または1なら費用は0です。長さ2以上では次の式で最小費用を計算します。

$$
D(l,r) = w(l,r) + \min_{l<k<r}\{D(l,k)+D(k,r)\}.
$$

`cost(l, r)` が $w(l,r)$ を返す関数です。任意の $0\le a\le b\le c\le d\le n$ に対して次の2条件を満たす必要があります。

- 区間包含による単調性 $w(b,c)\le w(a,d)$
- 四辺形不等式 $w(a,c)+w(b,d)\le w(a,d)+w(b,c)$

`size` を $n$、`max_length` を $L$ として、$0\le L\le n$ です。省略時は $L=n$ にします。`cost` が $O(1)$ なら構築は $O(nL+1)$ 時間・領域です。費用と途中の加算結果は `std::int64_t` の範囲に収まることが前提です。表が確保可能な要素数を超える場合は `std::length_error` を送出します。

## `size`

列の長さを $O(1)$ 時間で返します。

## `max_length`

計算した最大区間長を $O(1)$ 時間で返します。

## `get`

区間の最小費用を $O(1)$ 時間で返します。

## `split`

最小費用になる分割位置を $O(1)$ 時間で返します。同点の候補があれば最大の位置を選びます。区間長が0または1なら `std::nullopt` です。

`get(l, r)` と `split(l, r)` の添字は0始まりで、$0\le l\le r\le n$、$r-l\le L$ です。検査が有効なら、不正な構築寸法・区間・加算にはそれぞれ `std::invalid_argument`・`std::out_of_range`・`std::overflow_error` を送出します。

## 使用例

```cpp
const std::array<std::int64_t, 4> prefix{0, 1, 3, 6};
const sirkit::KnuthIntervalDP dp(3, [&](int l, int r)
{
    return prefix[r] - prefix[l];
});
assert(dp.get(0, 3) == 9 && dp.split(0, 3) == 2);
```

[実装](../Algorithm/Knuth_Interval_DP.cpp)・[使用例の全体](../Examples/knuth_interval_dp_example.cpp)・[Verify](../Verify/current/submissions/knuth_interval_dp.cpp)
