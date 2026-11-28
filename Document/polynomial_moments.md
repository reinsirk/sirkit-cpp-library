# 重み付きべき乗和と指数関数との合成

- `weighted_power_sums(points, weights, n)` は、$k=0,\ldots,n-1$ の $\sum_i \text{weights}[i]\text{points}[i]^k$ を返します。点と重みは同じ長さの整数ベクトルです。$0^0=1$ とします。
- `exponential_composition(f, scale, n, shift)` は、多項式 $f$ を $e^{\text{scale}\,x}+\text{shift}$ に代入した結果の先頭 `n` 係数を返します。`shift` の既定値は0です。
- 結果はどちらも法 $998244353$ の [FormalPowerSeries](formal_power_series.md) です。入力 $m$ 要素、出力 $n$ 係数で $O((m+n+1)\log^2(m+n+2))$ 時間、$O(m+n+1)$ 領域です。`n` と $f$ の係数数は $2^{22}$ 以下です。

たとえば点 `[2,3]` と重み `[4,5]` を指定したとき、最初の3項は $9,23,61$ です。

[使用例](../Examples/polynomial_moments_example.cpp) ／ [Verify](../Verify/current/submissions/polynomial_moments_yuki1875.cpp)
