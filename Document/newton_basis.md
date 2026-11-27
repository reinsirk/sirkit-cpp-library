# Newton基底への変換

単項式基底 $1,x,x^2,\ldots$ と、点 $p_j$ で定まるNewton基底 $1,x-p_0,(x-p_0)(x-p_1),\ldots$ の間で係数列を変換します。

```cpp
std::vector<std::uint32_t> f{1, 2, 1}, points{1, 2};
auto b = sirkit::monomial_to_newton_basis(f, points); // {4, 5, 1}
auto g = sirkit::newton_to_monomial_basis(b, points); // f
```

## `monomial_to_newton_basis`

```text
std::vector<std::uint32_t> monomial_to_newton_basis(
    std::span<const std::uint32_t> coefficients,
    std::span<const std::uint32_t> points);
```

単項式基底の係数列をNewton基底へ変換します。法998244353。$N$ 個の係数に $N-1$ 個の点を渡します。点は重複しても構いません。 $N=0$ の場合は点も空です。係数列の長さは変わりません。$N \leq 2^{20}$。係数数が上限を超えると `std::length_error`、点が必要数と異なると `std::invalid_argument` を送出します。 $O(N\log^2(N+1))$ 時間・$O(N\log(N+1)+1)$ 領域です。

## `newton_to_monomial_basis`

```text
std::vector<std::uint32_t> newton_to_monomial_basis(
    std::span<const std::uint32_t> coefficients,
    std::span<const std::uint32_t> points);
```

Newton基底の係数列を単項式基底へ変換します。入力と計算量は上と同じです。

単独貼り付け版に [形式的冪級数](formal_power_series.md) を同梱しています。

参考文献 [Bostan・Schost, Polynomial evaluation and interpolation on special sets of points, §3.1](https://mathexp.eu/bostan/publications/BoSc05.pdf)。

[使用例](../Examples/newton_basis_example.cpp) ／ [Verify](../Verify/current/submissions/conversion_from_monomial_basis_to_newton_basis.cpp)
