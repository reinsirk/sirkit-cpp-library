# BinaryMatrix

[BinaryMatrix](../Math/Binary_Matrix.cpp)は0/1の行列です。要素の和をXOR、積をANDとして計算するGF(2)上で、行列積・階数・逆行列・連立方程式を扱います。Mintは不要です。[使用例](../Examples/binary_matrix_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `BinaryMatrix(rows=0,cols=0)` | 指定した形の全0行列を作る。0×n・n×0も形を保持する。 |
| `rows()` / `cols()` / `empty()` | 行数 / 列数 / 要素数が0か。 |
| `test(r,c)` / `a(r,c)` | 要素をboolで返す。 |
| `set(r,c,value=true)` / `flip(r,c)` | 要素を変更し、自身への参照を返す。 |
| `row_words(r)` | 行の読み取り専用word span。word iのbit jは列64i+j。 |
| `swap_rows(a,b)` / `xor_rows(target,source,first_col=0)` | 行交換 / 指定列以降だけXORする。自己XORも可。 |
| `+` / `^` / `+=` / `^=` | 同形行列のGF(2)和。`==`は形と値を比較する。 |
| `transpose()` / `*` / `multiply(other)` / `*=` | 転置 / 行列積。自己積代入も可。 |
| `identity(n)` / `pow(k)` | n次単位行列 / 正方行列の非負整数乗。指数0は単位行列。 |
| `rank()` / `determinant()` | 階数 / 正方行列の行列式bool。0×0の行列式は1。 |
| `inverse()` | 逆行列のoptional。非正則ならnullopt、0×0は空の単位行列。 |
| `solve(rhs)` | Ax=rhsのoptional<LinearSolution>。不能ならnullopt。rhsは行数個のuint8_tの0/1。 |
| `reset(rows=0,cols=0)` / `clear()` / `swap(other)` | 全0への再初期化 / 0×0へ戻す / O(1)で交換する。 |

```cpp
BinaryMatrix a(2,3);
a.set(0,0).set(0,2).set(1,1).set(1,2);
auto result = a.solve(std::vector<std::uint8_t>{1,0});
// 特解は(1,0,0)、核の基底は(1,1,1)。
```

解の`particular`は列数個の特解、`kernel`は各行が核の基底となる(列数−rank)行の行列、`pivot_columns`は消去のピボット列です。任意の解は、特解と核基底の任意の組合せをXORして作れます。基底の具体的な形に依存せず使ってください。

0行では全変数が自由となり、特解は全0・核は単位行列です。0列ではrhsが全0の場合だけ解を持ちます。最初の非0で終了するため、後続の未参照rhsまで検査する保証はありません。

## 計算量・所有と検査

R行C列、W=ceil(C/64)、r=min(R,C)とします。行をまとめて1本のvectorで保持し、格納はR*W語です。構築・コピーはO(1+RW)、要素操作はO(1)、行操作は最悪O(W)、転置はO(1+RC)です。

R×MとM×Kの積はO(1+MK+R ceil(K/64)+RK ceil(M/64))時間で、転置と結果の2行列を追加確保します。rankの消去はO(1+RC+RrW)時間、inverseは正方行列で同規模です。solveはこれにO(R+C+C(C−rank))時間を追加し、核出力に(C−rank)ceil(C/64)語、特解・ピボット列にO(C)領域を使います。powはO(log(k+1))回の行列積です。

全てのconst計算は入力と既存viewを保ちます。コピーは独立し、コピー代入・reset・積代入とLinearSolutionのコピー代入は、確保失敗時に旧内容を維持します。move/swapはO(1)、move元は行列なら0×0、解なら全フィールド空です。row_wordsのviewは成功したreset・clear・代入・所有権移動まで有効で、通常の要素変更は同じ領域に反映されます。

`SKIP_RUNTIME_CHECKS`は番号・形・rhs長と参照したrhsの0/1検査を除きます。内部容量積の保護、0次元・0係数の判定は維持します。bit kernelは[DynamicBitset](dynamic_bitset.md)と独立ガードで共有し、[生成器](../Scripts/generate_binary_matrix.py)で単独貼付版へ同期します。`SIRKIT_DISABLE_SIMD`で追加のCPU判定と専用経路を除外できます。

VerifyはLibrary Checkerの[積](https://judge.yosupo.jp/problem/matrix_product_mod_2)・[階数](https://judge.yosupo.jp/problem/matrix_rank_mod_2)・[行列式](https://judge.yosupo.jp/problem/matrix_det_mod_2)・[逆行列](https://judge.yosupo.jp/problem/inverse_matrix_mod_2)・[連立方程式](https://judge.yosupo.jp/problem/system_of_linear_equations_mod_2)です。行操作・累乗・境界・所有権は独立ローカル参照で補います。
