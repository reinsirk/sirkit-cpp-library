# 行列と体上の消去

[Matrix](../Math/Matrix.cpp)は行列の加減・積・累乗、[Gaussian Elimination](../Math/Gaussian_Elimination.cpp)は階数・行列式・逆行列・連立一次方程式を扱います。後者はMatrixも同梱し、どちらも単独で貼り付けられます。[行列の使用例](../Examples/matrix_example.cpp)、[消去の使用例](../Examples/gaussian_elimination_example.cpp)を参照してください。

## 行列のAPI

| API | 動作 |
| --- | --- |
| `Matrix<T>(r,c,value=T{0})` | r行c列を指定値で初期化する。行リスト`{{...},...}`でも構築可。 |
| `Matrix<T>(r,c,vector<T>)` | r*c要素を値で受け取り所有する。moveなら領域を引き継ぐ。 |
| `rows()` / `cols()` / `size()` / `empty()` | 行数 / 列数 / 要素数 / 要素数0か。 |
| `a(r,c)` / `row(r)` / `values()` | 要素の参照 / 行のspan / 全要素のspan。 |
| `+` / `-` / `+=` / `-=` | 同形行列の加減。`==`は形と値を比較する。 |
| `*` / `*=` / `transposed()` | 行列積 / 積への置換 / 転置の独立コピー。 |
| `MatrixIdentity<T>(n)` / `MatrixPower(a,k)` | 単位行列 / 正方行列の非負整数乗。指数0は単位行列。 |
| `MatrixApply(a,x)` | 行列と列ベクトルの積をvectorで返す。 |
| `assign(r,c,value)` / `swap(other)` | 一様行列へ置換 / 内容を交換する。 |

```cpp
Matrix<std::int64_t> a{{1,2}, {3,4}}, b{{5,6}, {7,8}};
auto product = a * b; // {{19,22}, {43,50}}
auto x = MatrixApply(a, std::vector<std::int64_t>{1,2}); // {5,11}
auto power = MatrixPower(a, 0); // 2次単位行列。
```

Tは0/1から構築でき、必要な加減乗算とコピーを提供する型です。整数でも使えますが、中間計算を含むオーバーフローや浮動小数点誤差は利用側で管理します。`bool`は使わず、0/1行列には[BinaryMatrix](binary_matrix.md)も使えます。Mintは利用側で用意します。

## 消去のAPIと戻り値

| API | 動作 |
| --- | --- |
| `GaussianElimination(a,reduced=false)` | ピボットを1にした階段形を返す。trueならピボットの上も消去する。 |
| 結果の`matrix` / `pivot_columns` / `pivots` / `row_swaps` | 変形後の行列 / ピボット列 / 正規化前の値 / 行交換回数。`rank()`はピボット数。 |
| `MatrixRank(a)` / `MatrixDeterminant(a)` | 階数 / 正方行列の行列式。特異なら0、0×0の行列式は1。 |
| `MatrixInverse(a)` | 逆行列のoptional。非正則ならnullopt、0×0は空の単位行列。 |
| `SolveLinearSystem(a,b)` | a*x=bの解のoptional。不能ならnullopt。bは行数個。 |

解の`particular`は一解、`kernel`は各行が核の基底、`pivot_columns`はピボット列です。任意の解は`particular + t[0]*kernel.row(0) + ...`で表せます。0行では全変数が自由、0列ではbが零の場合だけ空の一解を持ちます。

**消去には正確な体の除算が必要**です。素数法Mintや有理数は使えますが、通常の整数型・一般の合成数法・浮動小数点向けの消去ではありません。最初の非零ピボットを使い、数値誤差への対策はしません。非可逆ピボットではdomain_errorを投げますが、係数型全体が体かを検証するものではありません。

## 計算量・所有と検査

係数の演算・コピーをO(1)とします。行列積(r,k)*(k,c)はO(r*c*(1+k))時間・O(r*c)追加領域、転置はO(r*c)、行列とベクトルの積はO(r*(1+c))です。n次正方行列の累乗はO(n³ log(k+2))時間・O(n²)追加領域です。

m行n列の消去はO(m*n*min(m,n))時間・O(m*n)領域、逆行列はO(n³)時間・O(n²)領域。連立方程式はO(m*(n+1)*(1+min(m,n))+n²)時間・O(m*(n+1)+n²)領域で、核基底の出力も含みます。

0行・0列も形を保ちます。行列は連続vectorを所有し、コピーは独立、move元は0×0で再利用可です。spanは非所有なので、代入・swap・破棄をまたいで保持しません。積は別領域で完成させ、`*=`も完成後に置換します。コピー代入・assignの確保失敗では元を保ちます。消去は値で受け取るため、通常のlvalue入力を変更しません。

`SKIP_RUNTIME_CHECKS`は形・添字等の入力検査を除きますが、容量積の保護・数学的な0判定・非可逆ピボットの保護は維持します。GCC/x86の対応CPUではuint32_tの積にAVX2を使い、結果は通常のunsigned演算と同じmod 2^32です。`SIRKIT_DISABLE_SIMD`で専用経路とCPU判定を除外できます。

用途の参考は[Nyaanの行列ライブラリ](https://nyaannyaan.github.io/library/matrix/matrix.hpp)、代数上の前提は[環と体の用語集](https://info.atcoder.jp/entry/algorithm_lectures/algebra_terms)です。実装と測定の記録は[Audit](../Audit/)に置きます。

VerifyはLibrary Checkerの[積](https://judge.yosupo.jp/problem/matrix_product)・[行列式](https://judge.yosupo.jp/problem/matrix_det)・[逆行列](https://judge.yosupo.jp/problem/inverse_matrix)・[階数](https://judge.yosupo.jp/problem/matrix_rank)・[累乗](https://judge.yosupo.jp/problem/pow_of_matrix)・[連立方程式](https://judge.yosupo.jp/problem/system_of_linear_equations)を使います。
