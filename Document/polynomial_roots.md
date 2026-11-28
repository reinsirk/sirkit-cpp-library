# 多項式の根

法 $998244353$ を使い、与えた多項式をゼロにする剰余をすべて求めます。

## `polynomial_roots`

```text
std::vector<std::uint32_t> polynomial_roots(
    FormalPowerSeries value, std::uint64_t seed = std::random_device{}())
```

相異なる根を昇順で返します。非零定数の答えは空です。零多項式では `std::domain_error` です。係数数 $L$ から末尾のゼロを除いた係数数 $N$ は $2^{21}$ 以下です。

因子を分けるときに乱数を使います。乱数で変わるのは分割の順序と実行時間です。同じ分割を再現したい場合は `seed` を指定します。期待 $O(L+N\log^3(N+1))$ 時間、$O(L)$ 領域です。

```cpp
const sirkit::FormalPowerSeries polynomial{0, -1, 1};
const auto roots = sirkit::polynomial_roots(polynomial); // {0, 1}
```

[使用例](../Examples/polynomial_roots_example.cpp) ／ [Verify](../Verify/current/submissions/polynomial_root_finding.cpp) ／ [多項式のGCD](polynomial_gcd.md) ／ [Cantor–Zassenhaus法](https://doi.org/10.1090/S0025-5718-1981-0606517-5)
