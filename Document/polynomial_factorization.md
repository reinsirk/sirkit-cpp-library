# 有限体上の多項式の因数分解

32ビットの素数 $p$ を法として、多項式を既約因子の積に分解します。重根を含む場合も各因子の重複度を求めます。

## `factor_polynomial`

```text
struct PolynomialFactor {
    std::vector<std::uint32_t> coefficients;
    std::size_t multiplicity;
};
struct PolynomialFactorization {
    std::uint32_t leading_coefficient;
    std::vector<PolynomialFactor> factors;
};
PolynomialFactorization factor_polynomial(
    std::span<const std::uint32_t> coefficients,
    std::uint32_t prime, std::uint64_t seed = std::random_device{}())
```

係数を定数項から順に渡します。入力は法 $p$ の剰余に直します。戻り値は元の最高次係数と、最高次係数が $1$ の相異なる既約因子です。各因子の係数は低次順、因子の列は係数列の辞書順です。

非零定数なら因子は空です。零多項式は `std::domain_error`、素数以外の法は `std::invalid_argument` です。乱数の分割を再現したい場合は `seed` を指定します。

入力の係数数を $L$、末尾ゼロを除いた係数数を $N$ として、期待 $O(L+N^3\log p+\sqrt p)$ 時間、$O(L)$ 領域です。

```cpp
const std::array<std::uint32_t, 3> coefficients{1, 1, 1};
const auto result = sirkit::factor_polynomial(coefficients, 3); // (x+2)^2
```

[使用例](../Examples/polynomial_factorization_example.cpp) ／ [Verify](../Verify/current/submissions/factorization_of_polynomials.cpp) ／ [参考資料](https://shoup.net/ntb/ntb-v2.pdf)
