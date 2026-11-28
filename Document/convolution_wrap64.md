# 法 2^64 の畳み込み

64ビット符号なし整数の自然な桁あふれと同じ規約で畳み込みます。

## `convolution_mod_2_64`

```text
std::vector<std::uint64_t> convolution_mod_2_64(
    std::span<const std::uint64_t> a, std::span<const std::uint64_t> b)
```

出力の添字 $k$ に $\sum_{i+j=k}a_i b_j$ を $2^{64}$ で割った余りを置きます。空の列があれば空です。出力の係数数 $N=|a|+|b|-1$ の上限は $2^{21}$ で、超えると `std::length_error` です。

$O(N\log(N+1))$ 時間、$O(N)$ 領域です。片方が32項以下なら筆算を使うので、時間は $O(|a||b|)$ です。

```cpp
const std::array<std::uint64_t, 2> a{UINT64_MAX, 2}, b{3, 4};
const auto c = sirkit::convolution_mod_2_64(a, b);
// c[1] = 2
```

[使用例](../Examples/convolution_wrap64_example.cpp) ／ [Verify](../Verify/current/submissions/convolution_mod_2_64.cpp) ／ [同梱NTT](ntt.md)
