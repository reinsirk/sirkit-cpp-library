# 指数が次数で変わる係数変換

- `exponential_coefficient_transform(f, step, n)` は、係数 $i$ を $[x^i]f(x)\exp(\text{step}\cdot i\cdot x)$ に置き換えた級数を返します。
- `inverse_exponential_coefficient_transform(g, step, n)` は、同じ `step` で変換する前の級数を復元します。
- 法は $998244353$ です。戻り値は $n$ 係数で、$0\le n\le 2^{18}$ とします。長さ $n$ を超える係数は使いません。

たとえば $f(x)=1+2x$、`step=1` なら、変換後の係数は先頭から $1,3,6$ です。確率や数え上げで、指数に取り出したい次数が入る式に使えます。

時間は $O(n\log^2(n+1))$、領域は $O(n\log(n+1))$ です。`step=0` なら切り詰めか0埋めだけを行います。

[使用例](../Examples/exponential_coefficient_transform_example.cpp) ／ [Verify](../Verify/current/submissions/exponential_coefficient_transform_agc076_d.cpp)
