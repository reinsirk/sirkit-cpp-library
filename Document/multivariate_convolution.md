# 打ち切り多変数畳み込み

多変数の多項式を掛け合わせ、指定した次数範囲の係数を求めます。

```cpp
const std::vector<std::size_t> dimensions{2, 2};
const std::vector<std::uint32_t> a{1, 2, 3, 4}, b{5, 6, 7, 8};
auto c = sirkit::multivariate_convolution(dimensions, a, b);  // {5, 16, 22, 60}
```

## `multivariate_convolution`

```text
std::vector<std::uint32_t> multivariate_convolution(
    std::span<const std::size_t> dimensions, std::span<const std::uint32_t> a,
    std::span<const std::uint32_t> b);
```

`dimensions[i]` は変数 $x_i$ の係数を保持する数です。積の各項は、いずれかの変数で次数がこの値以上なら除きます。法は998244353です。

幅を順に $n_0,n_1,\ldots$ とすると、$x_0^{i_0}x_1^{i_1}\cdots$ の係数は配列の `i0 + n0*i1 + n0*n1*i2 + …` 番目です。各幅は正で、係数列の長さは幅の積 $N\le 2^{18}$ です。幅の指定が空なら長さ1の定数として扱います。

軸数を $K$、幅2以上の軸数を1以上に切り上げた値を $D$ として、$O(K+DN(D+\log(N+1)))$ 時間・$O(DN)$ 領域です。

## 同梱の NTT・畳み込み

`NTT<Mod, Root>` の変換・畳み込み・二乗と、自由関数 `ConvolutionMod` も含みます。[NTTとConvolutionModのAPI](ntt.md#api)

[使用例](../Examples/multivariate_convolution_example.cpp) ／ [Verify](../Verify/current/submissions/multivariate_convolution.cpp)
