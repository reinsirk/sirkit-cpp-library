# BinaryMatrix

[BinaryMatrix](../Math/Binary_Matrix.cpp)は0/1の行列です。要素の和をXOR、積をANDとして計算するGF(2)上で、行列積・階数・逆行列・連立方程式を扱います。Mintは不要です。[使用例](../Examples/binary_matrix_example.cpp)を参照してください。

R行C列、r=min(R,C)、W=ceil(C/64)とします。積では(R,M)*(M,K)の形を用います。

## API

### BinaryMatrix::BinaryMatrix

```text
explicit BinaryMatrix(std::size_t rows = 0, std::size_t cols = 0);
BinaryMatrix(const BinaryMatrix&);
BinaryMatrix(BinaryMatrix&& other) noexcept;
```

指定した形の全0行列を作ります。0×n・n×0でも行数と列数を保持します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** R行C列にO(1+R ceil(C/64))時間・領域。移動構築・swapはO(1)。

### BinaryMatrix::operator=

```text
BinaryMatrix& operator=(const BinaryMatrix& other);
BinaryMatrix& operator=(BinaryMatrix&& other) noexcept;
```

コピーまたは移動で内容を置き換え、自身への参照を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** R行C列にO(1+R ceil(C/64))時間・領域。移動構築・swapはO(1)。

### BinaryMatrix::swap

```text
void swap(BinaryMatrix& other) noexcept;
friend void swap(BinaryMatrix& a, BinaryMatrix& b) noexcept;
```

内容と所有する領域を交換します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### BinaryMatrix::rows

```text
std::size_t rows() const noexcept;
```

行数を返します。

**制約:** なし。

**計算量:** O(1)。

### BinaryMatrix::cols

```text
std::size_t cols() const noexcept;
```

列数を返します。

**制約:** なし。

**計算量:** O(1)。

### BinaryMatrix::empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### BinaryMatrix::row_words

```text
std::span<const std::uint64_t> row_words(std::size_t row) const;
```

行を読み取り専用のword spanで返します。word iのbit jは列64i+jに対応します。

**制約:** 行添字が範囲内。viewは所有権移動・再構築まで有効。

**計算量:** O(1)。

### BinaryMatrix::test

```text
bool test(std::size_t row, std::size_t col) const;
```

要素をboolで返す。

**制約:** 行・列添字が範囲内。

**計算量:** O(1)。

### BinaryMatrix::operator()

```text
bool operator()(std::size_t row, std::size_t col) const;
```

指定した要素を返します。

**制約:** r<rows()、c<cols()。

**計算量:** O(1)。

### BinaryMatrix::set

```text
BinaryMatrix& set(std::size_t row, std::size_t col, bool value = true);
```

要素を変更し、自身への参照を返す。

**制約:** 更新する添字が範囲内。

**計算量:** O(1)。

### BinaryMatrix::flip

```text
BinaryMatrix& flip(std::size_t row, std::size_t col);
```

要素を変更し、自身への参照を返す。

**制約:** 行・列添字が範囲内。

**計算量:** O(1)。

### BinaryMatrix::swap_rows

```text
void swap_rows(std::size_t a, std::size_t b);
```

指定した2行を交換します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** W=ceil(C/64)としてO(W)。

### BinaryMatrix::xor_rows

```text
void xor_rows(std::size_t target, std::size_t source, std::size_t first_col = 0);
```

指定列以降だけXORする。自己XORも可。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** W=ceil(C/64)としてO(W)。

### BinaryMatrix::identity

```text
static BinaryMatrix identity(std::size_t n);
```

n次単位行列を返します。

**制約:** n次行列を確保できる容量。

**計算量:** O(1+n ceil(n/64))時間・領域。

### BinaryMatrix::transpose

```text
BinaryMatrix transpose() const;
```

転置した行列を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1+RC)時間。

### BinaryMatrix::multiply

```text
BinaryMatrix multiply(const BinaryMatrix& other) const;
```

行列積を求めます。自己積の代入にも使えます。

**制約:** 左行列の列数と右行列の行数が同じ。

**計算量:** (R,M)*(M,K)にO(1+MK+R ceil(K/64)+RK ceil(M/64))時間。

### BinaryMatrix::operator*

```text
BinaryMatrix operator*(const BinaryMatrix& other) const;
```

積を返します。

**制約:** 左行列の列数と右行列の行数が同じ。

**計算量:** (R,M)*(M,K)にO(1+MK+R ceil(K/64)+RK ceil(M/64))時間。

### BinaryMatrix::operator*=

```text
BinaryMatrix& operator*=(const BinaryMatrix& other);
```

乗算して自身を更新します。

**制約:** 左行列の列数と右行列の行数が同じ。

**計算量:** (R,M)*(M,K)にO(1+MK+R ceil(K/64)+RK ceil(M/64))時間。

### BinaryMatrix::operator+=

```text
BinaryMatrix& operator+=(const BinaryMatrix& other);
```

同形行列のGF(2)上の和を自身に代入して自身への参照を返します。

**制約:** 両行列の行数・列数が同じ。

**計算量:** R行C列にO(1+R ceil(C/64))時間・領域。移動構築・swapはO(1)。

### BinaryMatrix::operator^=

```text
BinaryMatrix& operator^=(const BinaryMatrix& other);
```

同形行列のGF(2)上の和を自身に代入して自身への参照を返します。

**制約:** 両行列の行数・列数が同じ。

**計算量:** R行C列にO(1+R ceil(C/64))時間・領域。移動構築・swapはO(1)。

### BinaryMatrix::operator+

```text
friend BinaryMatrix operator+(BinaryMatrix a, const BinaryMatrix& b);
```

同形行列のGF(2)上の和を新しい行列で返します。

**制約:** 両行列の行数・列数が同じ。

**計算量:** R行C列にO(1+R ceil(C/64))時間・領域。移動構築・swapはO(1)。

### BinaryMatrix::operator^

```text
friend BinaryMatrix operator^(BinaryMatrix a, const BinaryMatrix& b);
```

同形行列のGF(2)上の和を新しい行列で返します。

**制約:** 両行列の行数・列数が同じ。

**計算量:** R行C列にO(1+R ceil(C/64))時間・領域。移動構築・swapはO(1)。

### BinaryMatrix::operator==

```text
bool operator==(const BinaryMatrix& other) const noexcept;
```

値と形が等しいかを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** R行C列にO(1+R ceil(C/64))時間・領域。移動構築・swapはO(1)。

### BinaryMatrix::pow

```text
BinaryMatrix pow(std::uint64_t exponent) const;
```

正方行列の非負整数乗を返します。指数が0なら単位行列を返します。

**制約:** 正方行列。powの指数は非負。

**計算量:** O(log(k+1))回の行列積。

### BinaryMatrix::rank

```text
std::size_t rank() const;
```

階数を返します。

**制約:** なし。

**計算量:** r=min(R,C)、W=ceil(C/64)としてO(1+RC+RrW)時間。

### BinaryMatrix::determinant

```text
bool determinant() const;
```

正方行列の行列式をboolで返します。0×0の行列式は1です。

**制約:** 正方行列。

**計算量:** r=min(R,C)、W=ceil(C/64)としてO(1+RC+RrW)時間。

### BinaryMatrix::inverse

```text
std::optional<BinaryMatrix> inverse() const;
```

逆行列をoptionalで返します。非正則ならnullopt、0×0なら空の単位行列を返します。

**制約:** 正方行列。

**計算量:** r=min(R,C)、W=ceil(C/64)としてO(1+RC+RrW)時間。

### BinaryMatrix::solve

```text
std::optional<LinearSolution> solve(std::span<const std::uint8_t> rhs) const;
```

Ax=rhsの解をoptional<LinearSolution>で返します。解がなければnulloptです。rhsは行数と同じ個数のuint8_tの0/1で指定します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 消去O(1+RC+RrW)に核の生成O(R+C+C(C-rank))を加える。

### BinaryMatrix::reset

```text
void reset(std::size_t rows = 0, std::size_t cols = 0);
```

全要素が0の行列に初期化し直します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** R行C列にO(1+R ceil(C/64))時間・領域。移動構築・swapはO(1)。

### BinaryMatrix::clear

```text
void clear() noexcept;
```

内容を空に戻します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** R行C列にO(1+R ceil(C/64))時間・領域。移動構築・swapはO(1)。

### LinearSolution::LinearSolution

```text
LinearSolution();
LinearSolution(std::vector<std::uint8_t> value, BinaryMatrix basis, std::vector<std::size_t> pivots);
LinearSolution(const LinearSolution&);
LinearSolution(LinearSolution&& other) noexcept;
```

空または指定した入力から構築します。コピーは独立、移動は所有権を移します。

**制約:** なし。

**計算量:** コピーはparticular・pivot_columns・kernelの保持量に比例。移動構築・swapはO(1)、代入は旧状態の破棄も加える。

### LinearSolution::operator=

```text
LinearSolution& operator=(const LinearSolution& other);
LinearSolution& operator=(LinearSolution&& other) noexcept;
```

コピーまたは移動で内容を置き換え、自身への参照を返します。

**制約:** なし。

**計算量:** コピーはparticular・pivot_columns・kernelの保持量に比例。移動構築・swapはO(1)、代入は旧状態の破棄も加える。

### LinearSolution::swap

```text
void swap(LinearSolution& other) noexcept;
friend void swap(LinearSolution& a, LinearSolution& b) noexcept;
```

内容と所有する領域を交換します。

**制約:** なし。

**計算量:** コピーはparticular・pivot_columns・kernelの保持量に比例。移動構築・swapはO(1)、代入は旧状態の破棄も加える。


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
