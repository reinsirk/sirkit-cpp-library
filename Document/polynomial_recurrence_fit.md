# 先頭項から多項式係数の漸化式を求める

`fit_polynomial_recurrence(terms, order, max_degree)` は、与えた項で $\sum_{j=0}^{\text{order}} P_j(t)a_{t+j}=0$ が成り立つ多項式の組を法 $998244353$ で求めます。

- `terms` は添字0からの `int64_t` の列です。`1 <= order < terms.size()` とします。
- 返る `P_j` の係数は定数項順です。最後の多項式が非零になる解がなければ `nullopt` です。入力区間を越えて項を求めるには、漸化式がその区間でも成り立ち、最後の多項式の値が0にならないことを確かめます。
- 項数 $N$、係数の個数 $K=(\text{order}+1)(\text{max_degree}+1)$ として $O(NK^2+K\log 998244353)$ 時間、$O(N+K^2)$ 領域です。

たとえば `[1,1,2,6,24,120]` から、$-(t+1)a_t+a_{t+1}=0$ が求まります。項を計算するときは [多項式係数の漸化式](polynomial_recurrence.md) に係数を渡せます。

[使用例](../Examples/polynomial_recurrence_fit_example.cpp) ／ [Verify](../Verify/current/submissions/polynomial_recurrence_fit_abc222_h.cpp)
