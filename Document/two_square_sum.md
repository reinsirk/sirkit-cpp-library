# 二平方和の列挙

非負整数 $n$ を2個の平方数の和で表す方法をすべて列挙します。

```cpp
auto pairs = sirkit::two_square_sum(25);
// (0, 5), (3, 4), (4, 3), (5, 0)
```

## `two_square_sum`

```text
std::vector<std::pair<std::uint64_t, std::uint64_t>>
two_square_sum(std::uint64_t n);
```

$x^2+y^2=n$ となる非負整数の順序付き組 $(x,y)$ を辞書順に返します。たとえば $(3,4)$ と $(4,3)$ は別の組です。$n=0$ は $(0,0)$ のみ。表せなければ空です。 64ビット因数分解の後、$p\equiv1\pmod4$ の各素因数をCornacchia法で分けます。素因数 $p$ ごとに平方剰余でない候補を $g_p$ 個試すと $O(g_p\log p)$ 回の剰余演算。 これらの素因数の指数を $e_p$、$t=\prod_p(e_p+1)$ とすると、組の列挙と整列は $O(t\log(t+1))$ 時間・$O(t)$ 領域です。

## 同梱の64ビット因数分解

`PollardRho64` と、因数分解・約数・トーティエント関数などの自由関数も使えます。[PollardRho64と自由関数のAPI](pollard_rho.md#api)

[使用例](../Examples/two_square_sum_example.cpp) ／ [Verify](../Verify/current/submissions/two_square_sum.cpp)
