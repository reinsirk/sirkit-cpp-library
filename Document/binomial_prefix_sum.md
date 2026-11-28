# 二項係数の累積和

各問い合わせ $(n,k)$ について、$\binom{n}{0}+\cdots+\binom{n}{k}$ を求めます。異なる $n$ と $k$ が多数あるときは、Moのアルゴリズムの順序で処理し、直前の和を更新します。

## `binomial_prefix_sums`

```text
template <std::uint32_t Mod = 998244353>
std::vector<std::uint32_t> binomial_prefix_sums(
    std::span<const std::pair<std::uint32_t, std::uint32_t>> queries)
```

答えを `Mod` で割った余りを入力順に返します。`Mod` は奇素数で、各 `n` は `Mod` 未満です。`k >= n` なら $2^n$ を返します。最大の `n` を $N$、問い合わせ数を $Q$ とすると、$O(N+Q\log(Q+1)+N\sqrt{Q})$ 時間・$O(N+Q)$ 領域です。

```cpp
const std::vector<std::pair<std::uint32_t, std::uint32_t>> queries{{4, 2}, {5, 4}};
const auto sums = sirkit::binomial_prefix_sums(queries); // {11, 31}
```

[使用例](../Examples/binomial_prefix_sum_example.cpp) ／ [Verify](../Verify/current/submissions/prefix_sum_of_binomial_coefficient.cpp)
