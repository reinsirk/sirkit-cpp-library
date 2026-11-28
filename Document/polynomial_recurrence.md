# 係数が多項式で変わる漸化式の第k項

`polynomial_recurrence_term(initial, coefficients, index)` は、$\sum_{j=0}^{d}P_j(t)a_{t+j}=0$ を満たす数列の第 `index` 項を法 $998244353$ で求めます。

- `initial` は $a_0,\dots,a_{d-1}$。$d\ge1$ です。`coefficients[j]` に $P_j$ の `int64_t` 係数を定数項から並べます。
- 項を進める範囲のすべての $t$ で $P_d(t)\ne0$ が必要です。積でこの条件を調べ、0があれば `std::domain_error` になります。係数の正規化後の次数は256以下です。
- `index<d` は $O(1)$。それ以外は $d+1$ 行の [多項式行列積](polynomial_matrix_product.md) を $\text{index}-d+1$ 回分計算します。

たとえば $-(t+1)a_t+a_{t+1}=0$ と $a_0=1$ を指定すると、得られる数列は階乗です。

[使用例](../Examples/polynomial_recurrence_example.cpp) ／ [Verify](../Verify/current/submissions/polynomial_recurrence_abc222_h.cpp)
