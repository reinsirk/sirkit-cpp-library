# 三重対角行列を累乗したときの第0列

`tridiagonal_power_first_column(exponent, lower, diagonal, upper)` は、三重対角行列 $T$ の $\text{exponent}$ 乗の第0列を法 $998244353$ で返します。隣りの状態にだけ遷移するDPで、状態0から多数回進めたいときに使えます。

- 行数は `diagonal.size()`。下対角の `lower[i]` は $T_{i+1,i}$、上対角の `upper[i]` は $T_{i,i+1}$ です。どちらも主対角より1要素少ない列です。主対角が空なら、両方を空にします。
- 係数は負の `int64_t` も使えます。行列が空なら空の列、指数が0なら第0単位ベクトルを返します。
- $s=\min(n,\text{exponent}+1)$ は $2^{20}$ 以下。計算量は $O(n+s\log(s+1)(\log(s+1)+\log(\text{exponent}+2)))$ 時間、$O(n+s(\log(s+1)+\log(\text{exponent}+2)))$ 領域です。

たとえば `lower={1}`、`diagonal={1,1}`、`upper={1}` の行列なら、2乗の第0列は `{2,2}` です。

[使用例](../Examples/tridiagonal_matrix_power_example.cpp) ／ [Verify](../Verify/current/submissions/tridiagonal_matrix_power_arc228_e.cpp)
