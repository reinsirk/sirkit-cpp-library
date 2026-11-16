# 64ビット整数の因数分解

`PollardRho64`は`std::uint64_t`全域の整数を因数分解します。`PollardRho64(seed)`で生成し、seedは省略できます。

## API

### PollardRho64::PollardRho64

```text
explicit PollardRho64(std::uint64_t seed = 0x8b54e7163d20a9c1ULL) noexcept;
```

乱数状態だけを持つ64bit因数分解器。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### PollardRho64::is_prime

```text
static bool is_prime(std::uint64_t n) noexcept;
```

素数ならtrueを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(log(n+1))回の剰余乗算。

### PollardRho64::prime_factors

```text
std::vector<std::uint64_t> prime_factors(std::uint64_t n);
```

重複込みの素因数を昇順のvectorで返します。

**制約:** n>=1。n=0は検査省略時もdomain_error。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### PollardRho64::factorize

```text
std::vector<std::pair<std::uint64_t, int>> factorize(std::uint64_t n);
```

素数と指数の組を昇順で返します。

**制約:** n>=1。n=0は検査省略時もdomain_error。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### PollardRho64::divisors

```text
std::vector<std::uint64_t> divisors(std::uint64_t n, bool sorted = true);
```

正の約数を返します。sorted=trueなら昇順です。

**制約:** n>=1。n=0は検査省略時もdomain_error。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### PollardRho64::divisor_count

```text
std::uint64_t divisor_count(std::uint64_t n);
```

正の約数の個数を返します。

**制約:** n>=1。n=0は検査省略時もdomain_error。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### PollardRho64::totient

```text
std::uint64_t totient(std::uint64_t n);
```

Eulerのφ関数を返します。

**制約:** n>=1。n=0は検査省略時もdomain_error。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### PollardRho64::mobius

```text
int mobius(std::uint64_t n);
```

メビウス関数を返します。

**制約:** n>=1。n=0は検査省略時もdomain_error。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### PrimeFactors64

```text
std::vector<std::uint64_t> PrimeFactors64(std::uint64_t n);
```

一時分解器で重複込みの素因数列を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### Factorize64

```text
std::vector<std::pair<std::uint64_t, int>> Factorize64(std::uint64_t n);
```

一時分解器で素因数と指数の組を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### Divisors64

```text
std::vector<std::uint64_t> Divisors64(std::uint64_t n, bool sorted = true);
```

一時分解器で正の約数列を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### DivisorCount64

```text
std::uint64_t DivisorCount64(std::uint64_t n);
```

一時分解器で約数の個数を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### EulerPhi64

```text
std::uint64_t EulerPhi64(std::uint64_t n);
```

一時分解器でEulerのφ関数を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。

### Mobius64

```text
int Mobius64(std::uint64_t n);
```

一時分解器でメビウス関数を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 因数pの分離に期待O(sqrt(p))回の剰余乗算。約数D個の列挙は分解後O(D)、整列はO(D log(D+1))。


`PrimeFactors64`・`Factorize64`・`Divisors64`・`DivisorCount64`・`EulerPhi64`・`Mobius64`は、一時分解器を使う同名操作です。`Divisors64(n,sorted=true)`以外の引数はnだけです。

## 前提と計算量

素数判定以外はn>=1が必要です。n=0は検査省略時も`domain_error`です。n=1では因数列は空、約数は{1}、φとμは1です。

因数pを分離する期待計算量はO(sqrt(p))回の剰余乗算で、確定的な時間上界は保証しません。分解後の約数列挙はD個にO(D)、整列はO(D log(D+1))、個数・φ・μは素因数数に比例します。

各分解器は独立した擬似乱数状態を持ち、入力や返却済みの結果を変更しません。

```cpp
PollardRho64 engine;
auto factors = engine.factorize(360); // {{2,3},{3,2},{5,1}}。
auto phi = engine.totient(36);        // 12。
auto count = engine.divisor_count(36); // 9。
```

[完全な使用例](../Examples/pollard_rho_example.cpp)・[内部処理と検証の記録](../Audit/library-docs-concise-math-geometry-2026-11-15.json)
