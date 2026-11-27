# 随伴行列（余因子行列の転置）

正方行列の各余因子を転置して並べます。逆行列を持たない行列にも使えます。

```cpp
using Mint = atcoder::modint998244353;
const sirkit::Matrix<Mint> a{{1, 2}, {3, 4}};
auto adj = sirkit::MatrixAdjugate(a);  // {{4, -2}, {-3, 1}} を法で表した値
```

## `MatrixAdjugate`

```text
template <class T>
Matrix<T> MatrixAdjugate(const Matrix<T> &matrix);
```

余因子 `C[i][j]` を戻り値の `[j][i]` に置きます。係数型には素数法の `modint` など、正確な体の型を渡します。0行0列は空行列、1行1列は係数にかかわらず `{1}` です。$n$ 行 $n$ 列の正方行列に対し、体の演算を定数時間として $O(1+n^3)$ 時間・$O(n^2)$ 追加領域です。

## 同梱の行列操作

`Matrix<T>` の構築・行列積・累乗と、階数・行列式・逆行列・連立方程式の操作も含みます。[MatrixのAPI](pages/matrix.md#api) ／ [ガウス消去のAPI](pages/gaussian_elimination.md#api)

[使用例](../Examples/adjugate_matrix_example.cpp) ／ [Verify](../Verify/current/submissions/adjugate_matrix.cpp)
