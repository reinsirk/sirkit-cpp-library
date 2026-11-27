# 離散対数

`base` を何乗すれば、`modulus` で割った余りが `target` と等しくなるかを求めます。

```cpp
const auto exponent = sirkit::discrete_logarithm_mod(2, 4, 12); // 2
```

## `discrete_logarithm_mod`

```text
std::optional<std::uint32_t> discrete_logarithm_mod(
    std::uint32_t base, std::uint32_t target, std::uint32_t modulus);
```

`base^k ≡ target (mod modulus)` を満たす最小の非負整数 `k` を返します。解がなければ `std::nullopt` です。底と目標値は法で割った余りに直します。底と法が互いに素でなくても使えます。指数0の値は底によらず1とし、法が1なら答えは0です。

法を `M` として、`1 ≤ M < 2^32` が必要です。Baby-step Giant-step 法を使います。ハッシュ表の操作を平均 `O(1)` としたときの時間は `O(√M)` です。衝突が集中する最悪時は `O(M)` 時間で、追加領域は `O(√M)` です。検査が有効な場合、法が0なら `std::invalid_argument` を送出します。

[使用例](../Examples/discrete_logarithm_example.cpp) ／ [検証問題](https://judge.yosupo.jp/problem/discrete_logarithm_mod)
