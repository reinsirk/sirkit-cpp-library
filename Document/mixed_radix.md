# 混合基数の桁と整数の変換

桁ごとに異なる基数を指定し、桁の列と多倍長整数を相互に変換します。たとえば基数が $(3,4,2)$ で桁が $(1,2,1)$ なら、値は $(1\times4+2)\times2+1=13$ です。

## 構築

```text
template <unsigned Radix = 10>
class MixedRadix;
explicit MixedRadix(std::span<const std::uint32_t> radices)
```

基数を上位桁から順に渡します。基数は 1 以上です。`Radix` は結果の `BigInteger` に使う 10 または 16 です。基数の個数を $n$、基数の積の桁数を $D$ として、$O(n+D\log(D+1)\log(n+1))$ 時間・$O(n+D\log(n+1))$ 領域です。

## `capacity`

```text
const BigInteger<Radix> &capacity() const noexcept
```

基数の積を返します。表現できる値は 0 以上この値未満です。参照はオブジェクトが生存している間使えます。$O(1)$ 時間です。

## `encode`

```text
BigInteger<Radix> encode(std::span<const std::uint32_t> digits) const
```

上位から並べた桁を整数にします。対応する基数より小さい桁を、基数と同じ個数だけ渡します。時間は $O(n+D\log(D+1)\log(n+1))$ です。

## `decode`

```text
std::vector<std::uint32_t> decode(BigInteger<Radix> value) const
```

表現可能な値を上位から並んだ桁に戻します。範囲外は `std::out_of_range` です。時間は $O(n+D\log^2(D+1)\log(n+1))$ です。

```cpp
const std::array<std::uint32_t, 3> radices{3, 4, 2}, digits{1, 2, 1};
const sirkit::MixedRadix format(radices);
const auto value = format.encode(digits); // 13
```

[使用例](../Examples/mixed_radix_example.cpp) ／ [Verify](../Verify/current/submissions/permutation_rank_decimal_yuki696.cpp) ／ [同梱の多倍長整数](big_integer.md)
