# 多項式の拡張GCDと逆元

法 $998244353$ の係数を持つ多項式に対し、最大公約数、Bézout係数、法多項式に関する逆元を求めます。

## `polynomial_extended_gcd`

```text
struct PolynomialBezout {
    FormalPowerSeries gcd;
    FormalPowerSeries first;
    FormalPowerSeries second;
};
PolynomialBezout polynomial_extended_gcd(FormalPowerSeries a, FormalPowerSeries b)
```

`first * a + second * b == gcd` を満たす係数を返します。GCD の最高次係数は 1 です。$a=b=0$ のときは3つとも零多項式です。末尾のゼロを除いた係数数の上限は $2^{21}$ です。

## `polynomial_inverse_mod`

```text
std::optional<FormalPowerSeries> polynomial_inverse_mod(FormalPowerSeries value, FormalPowerSeries modulus)
```

`value * h ≡ 1 (mod modulus)` を満たす逆元 $h$ を返します。逆元は法多項式より低次数です。互いに素でなければ `std::nullopt` です。法が非零定数なら、逆元の代表元として零多項式を返します。零多項式を法とした場合は `std::domain_error` です。

末尾のゼロを除いた係数数の大きい方を $N$、入力の係数数の合計を $L$ とします。どちらの操作も $O(L+N\log^2(N+1))$ 時間、$O(L)$ 領域です。

```cpp
const sirkit::FormalPowerSeries x{0, 1}, modulus{-1, 0, 1};
const auto inverse = sirkit::polynomial_inverse_mod(x, modulus); // x
```

[使用例](../Examples/polynomial_gcd_example.cpp) ／ [Verify](../Verify/current/submissions/inv_of_polynomials.cpp) ／ [同梱FPS](formal_power_series.md) ／ [half-GCDの論文](https://arxiv.org/abs/2212.12389)
