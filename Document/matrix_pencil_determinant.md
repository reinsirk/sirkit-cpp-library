# 一次式を成分に持つ行列の行列式

`matrix_pencil_determinant<Mod>(constant, linear)` は、行列 $A+xB$ の行列式を、$x$ の多項式として求めます。`constant` は $A$、`linear` は $B$ です。

- 両方とも同じ大きさ $n$ の正方行列です。負の `int64_t` 係数を使えます。法 `Mod` は素数で、既定値は $998244353$。
- 返り値は長さ $n+1$ で、定数項から並びます。$B$ が特異でも計算できます。行列式が恒等的に0なら全項0、空行列なら `{1}` です。
- $O(n^3+n\log(\text{Mod})+1)$ 時間、$O(n^2+1)$ 領域です。

たとえば $A=\begin{pmatrix}1&2\\3&4\end{pmatrix}$、$B=\begin{pmatrix}1&0\\0&0\end{pmatrix}$ の行列式は $4x-2$ です。

[使用例](../Examples/matrix_pencil_determinant_example.cpp) ／ [Verify](../Verify/current/submissions/matrix_pencil_determinant_yuki1303.cpp)
