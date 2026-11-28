# q二項係数（素数法）

$q$ 二項係数は通常の二項係数を多項式に拡張したものです。$q$ と法を固定して、複数の $(n,k)$ に答えるときに使います。

## `PrimeQBinomial`

```text
PrimeQBinomial(std::uint32_t prime, std::uint32_t q, std::uint32_t max_n)
```

`prime` は素数で、`q` と `max_n` は `prime` 未満です。`max_n` までを前計算します。$q^i=1$ となる箇所では $q$-Lucasの定理を使います。構築は $O(\mathrm{max\_n}+\log(\mathrm{prime}))$ 時間・$O(\mathrm{max\_n}+1)$ 領域です。

## `COM`

```text
std::uint32_t COM(std::uint32_t n, std::uint32_t k) const
```

$\binom{n}{k}_q$ を `prime` で割った余りを返します。`n` は `max_n` 以下で、`k > n` の答えは 0 です。$q=0$ では範囲内の係数は 1、$q=1$ では通常の二項係数になります。1 回 $O(1)$ です。

```cpp
const sirkit::PrimeQBinomial coefficients(998244353, 2, 6);
const auto value = coefficients.COM(6, 2); // 651
```

[使用例](../Examples/q_binomial_example.cpp) ／ [Verify](../Verify/current/submissions/q_binomial_coefficient_prime_mod.cpp)
