# 小さい素数を法とするスターリング数

添字が大きいスターリング数を、小さい素数 $p$ で割った余りとして求めます。同じ法のクエリには、構築した表を再利用できます。

## `first_kind`

```text
static SmallPrimeStirlingNumbers first_kind(std::uint32_t prime)
```

符号付き第1種の表を構築します。値は下降階乗 $x(x-1)\cdots(x-n+1)$ の $x^k$ の係数です。$p$ は 5000 以下の素数です。構築は $O(p^2)$ 時間・領域です。

## `second_kind`

```text
static SmallPrimeStirlingNumbers second_kind(std::uint32_t prime)
```

第2種の表を構築します。値は $n$ 個の要素を $k$ 個の空でない集合に分ける個数です。$p$ は 5000 以下の素数です。構築は $O(p^2)$ 時間・領域です。

## `get`

```text
std::uint32_t get(std::uint64_t n, std::uint64_t k) const noexcept
```

法で割った余りを $O(1+\log_p(n+1))$ 時間で返します。`get(0, 0)` は 1、$k>n$ なら 0 です。

## `modulus`

```text
std::uint32_t modulus() const noexcept
```

法 $p$ を $O(1)$ 時間で返します。

```cpp
const auto stirling = sirkit::SmallPrimeStirlingNumbers::first_kind(7);
auto value = stirling.get(4, 2); // 11 mod 7 = 4
```

[使用例](../Examples/small_prime_stirling_example.cpp) ／ [第1種のVerify](../Verify/current/submissions/stirling_number_of_the_first_kind_small_p_large_n.cpp) ／ [第2種のVerify](../Verify/current/submissions/stirling_number_of_the_second_kind_small_p_large_n.cpp)
