# 素数の個数

整数以下の素数の個数を数えます。ふるいの表に必要な領域は平方根に比例します。

```cpp
assert(sirkit::count_primes(100) == 25);
```

## `count_primes`

```text
std::uint64_t count_primes(std::uint64_t n);
```

$n$ 以下の素数の個数を返します。$n<2$ なら0です。$O(n^{3/4})$ 時間・$O(\sqrt n)$ 領域です。

[使用例](../Examples/prime_counting_example.cpp) ／ [Verify](../Verify/current/submissions/counting_primes.cpp)
