# 多項式の累積和

多項式 `f` の値 `f(0), f(1), …` を足した結果を、別の多項式 `g` で表します。

```cpp
const sirkit::FormalPowerSeries f{0, 1};  // f(x)=x
const auto g = sirkit::PolynomialPrefixSum(f);  // x(x-1)/2
```

## `PolynomialPrefixSum`

```text
FormalPowerSeries PolynomialPrefixSum(const FormalPowerSeries &f);
```

`g(0)=0` と `g(x+1)-g(x)=f(x)` を満たします。整数 $k\ge 0$ では $g(k)=\sum_{i=0}^{k-1}f(i)$ です。係数の法は998244353です。

入力が $n$ 係数なら出力は $n+1$ 係数です。$n\le 2^{22}$ とし、空の入力では `{0}` を返します。$O(1+n\log(n+1))$ 時間・$O(1+n)$ 領域です。

## 同梱の形式的冪級数・NTT

`FormalPowerSeries` の四則演算・微積分・精度を指定する演算と、`NTT<Mod, Root>`、`ConvolutionMod` も含みます。[FormalPowerSeriesのAPI](formal_power_series.md#api) ／ [NTTとConvolutionModのAPI](ntt.md#api)

[使用例](../Examples/polynomial_prefix_sum_example.cpp) ／ [Verify](../Verify/current/submissions/prefix_sum_of_polynomial.cpp) ／ [Bernoulli数の母関数](https://dlmf.nist.gov/24.2.E1)
