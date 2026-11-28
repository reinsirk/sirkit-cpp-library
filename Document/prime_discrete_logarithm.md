# 素数法を固定した離散対数

同じ素数法と原始根で離散対数を繰り返し求めます。Baby-step Giant-stepの前計算を問い合わせの間で共有します。

## 構築

```text
PrimeDiscreteLogarithm(std::uint32_t prime, std::uint32_t generator,
                       std::uint64_t expected_queries = 1)
```

素数 `prime` とその原始根 `generator` を指定します。`expected_queries` が大きいほど前計算する累乗の個数 $B$ を増やし、1 回の問い合わせを短くします。問い合わせ回数に上限はありません。構築は期待 $O(B+\sqrt{\mathrm{prime}})$ 時間、保持領域は $O(B)$ です。

## `log`

```text
std::uint32_t log(std::uint32_t value) const
```

$\mathrm{generator}^x\equiv \mathrm{value}\pmod{\mathrm{prime}}$ を満たす最小の非負整数 $x$ を返します。`value` は 1 以上 `prime` 未満です。期待 $O(\mathrm{prime}/B+1)$ 時間です。

```cpp
const sirkit::PrimeDiscreteLogarithm table(7, 3, 2);
const auto exponent = table.log(2); // 3^2 = 2 mod 7
```

[使用例](../Examples/prime_discrete_logarithm_example.cpp) ／ [Verify](../Verify/current/submissions/discrete_logarithm_fixed_mod.cpp)
