# 彩色多項式

隣接する頂点を異なる色で塗る方法の数を多項式で表します。色数を代入すると、その色数での塗り方が得られます。[本体](../Graph/Chromatic_Polynomial.cpp)

## `chromatic_polynomial`

```text
std::vector<std::uint32_t> chromatic_polynomial(
    int n, std::span<const std::pair<int, int>> edges);
```

頂点番号が $0$ から $n-1$ の無向グラフを受け取り、係数を定数項から $n$ 次の順に法 $998244353$ で返します。頂点数は $0\le n\le 20$ です。多重辺を許し、自己辺があれば全係数が $0$、頂点がなければ `{1}` になります。通常版では範囲外の端点に `std::out_of_range` を投げます。

独立集合への分割を色数ごとに数え、係数へ変換します。辺数を $m$ とすると時間は $O((n+1)^2 2^n+m)$、追加領域は $O((n+1)2^n)$ です。同梱する部分集合畳み込みと集合の指数・対数については[集合冪級数](set_power_series.md)を参照してください。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}};
const auto p = sirkit::chromatic_polynomial(3, edges);
// x(x-1)(x-2) = 2x - 3x^2 + x^3
```

[使用例](../Examples/chromatic_polynomial_example.cpp)・[Verify](../Verify/current/submissions/chromatic_polynomial.cpp)
