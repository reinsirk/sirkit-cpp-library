# 集合冪級数の合成と累乗の内積

集合ごとの係数をビットマスク順に持つ冪級数を扱います。積では $x_i^2=0$ とみなし、要素が重ならない組だけを加えます。係数は法 $998244353$ で計算します。

## `set_series_compose`

```text
std::vector<std::uint32_t> set_series_compose(
    std::span<const std::uint32_t> polynomial,
    std::span<const std::uint32_t> series)
```

一変数多項式に集合冪級数を代入した係数を返します。`polynomial` は定数項から順に渡します。級数の定数項に 0 以外も使えます。

## `set_series_power_projection`

```text
std::vector<std::uint32_t> set_series_power_projection(
    std::span<const std::uint32_t> series,
    std::span<const std::uint32_t> weights, std::size_t count)
```

$k=0,\ldots,\texttt{count}-1$ の順に、`series` の $k$ 乗と `weights` の係数ごとの積の総和を返します。`weights` は `series` と同じ長さです。

両方とも `series` の長さは $2^n$、$0\le n\le 20$ です。多項式の係数数または `count` を $M$ とすると、$O((n+1)^2 2^n+(n+1)M)$ 時間、$O((n+1)2^n+M)$ 領域です。

```cpp
const std::array<std::uint32_t, 3> f{1, 2, 3};
const std::array<std::uint32_t, 4> s{2, 3, 5, 7};
const auto answer = sirkit::set_series_compose(f, s); // {17, 42, 70, 188}
```

[使用例](../Examples/set_series_composition_example.cpp) ／ [合成のVerify](../Verify/current/submissions/polynomial_composite_set_power_series.cpp) ／ [累乗の内積のVerify](../Verify/current/submissions/power_projection_of_set_power_series.cpp) ／ [同梱の集合冪級数](set_power_series.md)
