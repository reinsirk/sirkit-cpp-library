# 上下限付きの広義単調増加列の数え上げ

位置ごとの上下限を指定して、$x_0\le x_1\le\cdots\le x_{N-1}$ を満たす整数列の個数を求めます。

## `count_bounded_nondecreasing_sequences`

```text
std::uint32_t count_bounded_nondecreasing_sequences(
    std::span<const std::uint32_t> lower,
    std::span<const std::uint32_t> upper)
```

各位置で `lower[i] <= x[i] < upper[i]` を満たす列を数え、法 $998244353$ での剰余を返します。上下限の長さ $N$ は同じです。空の列は1通り、条件が矛盾すれば0通りです。

条件が矛盾しないとき、$M=\texttt{upper.back()}-\texttt{lower.front()}$ とします。$N+M\le 2^{21}$ が必要です。時間は $O((N+M)\log^2(N+M+1))$、領域は $O(N+M)$ です。

```cpp
const std::array<std::uint32_t, 2> lower{0, 1}, upper{2, 3};
const auto count = sirkit::count_bounded_nondecreasing_sequences(lower, upper); // 4
```

[使用例](../Examples/bounded_nondecreasing_sequences_example.cpp) ／ [Verify](../Verify/current/submissions/number_of_increasing_sequences_between_two_sequences.cpp) ／ [同梱NTT](ntt.md)
