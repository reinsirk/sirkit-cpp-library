# 添字の積による畳み込み

2つの係数列を掛け、添字の積が同じ剰余になる項を足し合わせます。添字の法には素数か2冪を使えます。

```cpp
const std::vector<std::uint32_t> a{1, 2, 3}, b{4, 5, 6};
auto c = sirkit::multiplicative_convolution_prime(a, b);  // {35, 28, 27}
```

## `multiplicative_convolution_prime`

```text
std::vector<std::uint32_t> multiplicative_convolution_prime(
    std::span<const std::uint32_t> a, std::span<const std::uint32_t> b);
```

入力2列は同じ長さの素数 $p\le 2^{19}$ とします。`a[i]*b[j]` を `i*j % p` 番目へ足し、係数は法998244353で返します。原始根の候補を $g$ 個試すとき $O(p\log(p+1)+g\log^2(p+1))$ 時間・$O(p)$ 領域です。

## `multiplicative_convolution_power_of_two`

```text
std::vector<std::uint32_t> multiplicative_convolution_power_of_two(
    std::span<const std::uint32_t> a, std::span<const std::uint32_t> b);
```

長さ $n=2^e$、$0\le e\le20$ の2列を渡します。`a[i]*b[j]` を `i*j % n` 番目へ足し、係数は法998244353で返します。$O(n\log(n+1))$ 時間・$O(n)$ 領域です。

## 同梱の NTT・畳み込み

`NTT<Mod, Root>` の変換・畳み込み・二乗と、自由関数 `ConvolutionMod` も含みます。[NTTとConvolutionModのAPI](ntt.md#api)

[使用例](../Examples/multiplicative_convolution_example.cpp) ／ [Verify](../Verify/current/submissions/mul_modp_convolution.cpp)
