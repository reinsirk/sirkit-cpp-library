# 多項式行列を連続する引数で評価した積

`polynomial_matrix_interval_product(matrix, first, count)` は、行列 $A$ の各成分を多項式とみなし、$A(\text{first})A(\text{first}+1)\cdots A(\text{first}+\text{count}-1)$ を法 $998244353$ で返します。引数が増える順に右へ掛けます。

- 入力は正方行列です。成分ごとに `int64_t` の係数を定数項から並べます。空の係数列は零多項式。法で正規化した後の次数は256以下です。
- `first` は負でも指定できます。引数を法で割った余りだけで評価値が決まるため `count` は `uint64_t` の全域に対応します。0回なら単位行列です。
- $s$ を行数、$d$ を最大次数、$P=998244353$、$H=1+\sqrt{(d+1)\min(\text{count},P)}$ とします。係数 $C$ 個で $O(C+s^2(d+1)^2+s^2H\log(H+1)+s^3(H+\log(\text{count}+1)))$ 時間、$O(s^2(d+H+1))$ 領域です。

たとえば $A(x)=\begin{pmatrix}1&x\\0&1\end{pmatrix}$ に対し、`first=2, count=3` の積の右上成分は $2+3+4=9$ です。

[使用例](../Examples/polynomial_matrix_product_example.cpp) ／ [Verify](../Verify/current/submissions/polynomial_matrix_product_arc174_c.cpp)
