# 大きい列の剰余畳み込み

1回のNTTでは収まらない長さの列を分割し、法 `Mod` で畳み込みます。既定の法は `998244353` です。

## `convolution_mod_large<Mod, Root>(a, b, transform_length)`

`a` と `b` は `std::uint32_t` の列です。値を法 `Mod` で割った余りに直し、$c_k=\sum_{i+j=k}a_ib_j\pmod{\mathrm{Mod}}$ を返します。片方が空なら結果も空です。

`Mod` と `Root` の条件は `NTT` と共通です。変換長 `transform_length` は省略すると `NTT<Mod,Root>::max_length()` になり、指定する場合はその上限以下の2以上の2冪とします。変換長を大きくすると、分割数が減り、1回の変換が大きくなります。

| 入力 | 計算量 |
|---|---|
| 短い方が16要素以下 | $O(\lvert a\rvert\lvert b\rvert)$ 時間。 |
| 結果が1回のNTTに収まる | 通常の畳み込みと同じ。 |
| 分割が必要 | 変換長 $L$、分割数 $P,Q$ に対して $O((P+Q)L\log L+PQL)$ 時間、$O((P+Q)L)$ 領域。 |

```cpp
const std::vector<std::uint32_t> a{1, 2, 3}, b{4, 5};
const auto c = sirkit::convolution_mod_large(a, b); // {4, 13, 22, 15}
```

[使用例](../Examples/large_convolution_example.cpp) ／ [Verify](../Verify/current/submissions/convolution_mod_large.cpp) ／ [NTT](ntt.md)
