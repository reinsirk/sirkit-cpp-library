# 微分を含む多項式の遷移

- `polynomial_differential_operator_power(f,p,a,t,n)` は、$g\mapsto p(x)g'(x)+a(x)g(x)$ を $f$ に $t$ 回適用し、先頭 $n$ 係数を返します。
- 係数の法は $998244353$ です。$t+n\le2^{18}$ とします。実際に遷移する場合は $p(0)\ne0$ が必要です。
- $N=t+n$ として、時間は $O(N\log^2(N+1))$、領域は $O(N\log(N+1))$ です。$x^N$ 以上の項は結果に影響しません。

たとえば $f=x$、$p=x^2-1$、$a=0$ に3回適用すると、$2-8x^2+6x^4$ になります。

[使用例](../Examples/polynomial_differential_operator_example.cpp) ／ [Verify](../Verify/current/submissions/polynomial_differential_operator_yuki2583.cpp)
