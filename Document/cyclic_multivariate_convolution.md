# 多次元巡回畳み込み

各方向に周期がある格子で、2つの係数列を畳み込みます。方向 $i$ の長さを $N_i$ とし、座標 $x_i$ と $y_i$ の組は、その方向の $(x_i+y_i)\bmod N_i$ に加算します。周期境界のある格子での回数集計などに使えます。

## `cyclic_multivariate_convolution(dimensions, a, b, prime)`

`dimensions` は各方向の長さです。座標 $(x_0,x_1,\ldots)$ の係数は添字 $x_0+N_0x_1+N_0N_1x_2+\cdots$ に置きます。`a`・`b`・戻り値はそれぞれ長さ $N=\prod_i N_i$ の `std::uint32_t` 列です。係数は `prime` で割った余りを使います。

`prime` は素数で、各 $N_i$ は `prime - 1` の正の約数です。$N\le 2^{21}$。方向が0個の場合は1要素同士の積を返します。原始根の計算を除くと、$K$ 方向に対して $O(K+N\log(N+1))$ 時間、$O(N)$ 作業領域です。

```cpp
const std::array<std::size_t, 2> dimensions{2, 3};
const std::vector<std::uint32_t> a{1, 2, 3, 4, 5, 6}, shift{0, 0, 1, 0, 0, 0};
const auto result = sirkit::cyclic_multivariate_convolution(dimensions, a, shift, 7);
// {5, 6, 1, 2, 3, 4}
```

[使用例](../Examples/cyclic_multivariate_convolution_example.cpp) ／ [Verify](../Verify/current/submissions/multivariate_convolution_cyclic.cpp)
