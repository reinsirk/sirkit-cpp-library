# 等比数列と多項式の積の和

各項が `r^i * f(i)` となる数列の有限和や、無限和の有理式を法998244353で計算します。

```cpp
auto finite = sirkit::sum_exponential_powers(2, 2, 4); // 90
auto limit = sirkit::sum_exponential_powers_limit(2, 1); // 2
```

## `sum_exponential_polynomial`

```text
std::uint32_t sum_exponential_polynomial(
    std::span<const std::uint32_t> samples, std::uint64_t ratio, std::uint64_t n);
```

入力を $f(0),\ldots,f(d)$ とみなし、$\sum_{i=0}^{n-1}r^i f(i)$ を返します。$r^0=1$ とします。空入力は零多項式です。 非空のとき $d+1<998244353$。$O(d+1+\log(n+1))$ 時間・$O(d+1)$ 領域です。

## `sum_exponential_polynomial_limit`

```text
std::optional<std::uint32_t> sum_exponential_polynomial_limit(
    std::span<const std::uint32_t> samples, std::uint64_t ratio);
```

$\sum_{i=0}^{\infty} r^i f(i)$ の有理式を有限体で評価します。$r\equiv1$ のときは `std::nullopt`。入力の前提は有限和と同じで、$O(d+1)$ 時間・領域です。

## `sum_exponential_powers`

```text
std::uint32_t sum_exponential_powers(
    std::uint64_t ratio, std::size_t degree, std::uint64_t n);
```

$f(i)=i^d$ の有限和です。$0^0=1$。$d+1<998244353$。篩で必要なべき乗を作り、$O(d+1+\log(n+1))$ 時間・$O(d+1)$ 領域で答えます。

## `sum_exponential_powers_limit`

```text
std::optional<std::uint32_t> sum_exponential_powers_limit(
    std::uint64_t ratio, std::size_t degree);
```

$f(i)=i^d$ の無限和の有理式を有限体で評価します。$r\equiv1$ なら `std::nullopt`。$O(d+1)$ 時間・領域です。

[使用例](../Examples/exponential_polynomial_sum_example.cpp) ／ [有限和のVerify](../Verify/current/submissions/sum_of_exponential_times_polynomial.cpp) ／ [無限和のVerify](../Verify/current/submissions/sum_of_exponential_times_polynomial_limit.cpp)
