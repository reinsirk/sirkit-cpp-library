# 行列の消去・階数・連立一次方程式

行基本変形による階段形、階数・行列式・逆行列、連立一次方程式の一解と核の基底を求めます。`Matrix<T>`とその演算も同梱するため、このファイルだけで行列の構築・積・累乗も使えます。

Matrixの`size_type`は`std::size_t`、`value_type`はTです。

行列のr・cは行数・列数、kは積の中間次元、nは正方行列の次数です。係数演算・コピーをO(1)として数えます。

## API

### GaussianEliminationResult::rank

```text
std::size_t rank()const noexcept;
```

階数を返します。

**制約:** なし。

**計算量:** O(1)。

### GaussianElimination

```text
template<class T>GaussianEliminationResult<T> GaussianElimination(Matrix<T> matrix,bool reduced=false);
```

階段形または簡約階段形とピボット情報を返します。

**制約:** 係数Tは正確な体。整数・一般の合成数法・浮動小数点用ではない。

**計算量:** m行n列にO(m*n*min(m,n))時間・O(m*n)領域。

### MatrixRank

```text
template<class T>std::size_t MatrixRank(const Matrix<T>& matrix);
```

体上の行列の階数を返します。

**制約:** 係数Tは正確な体。

**計算量:** m行n列にO(m*n*min(m,n))時間・O(m*n)領域。

### MatrixDeterminant

```text
template<class T>T MatrixDeterminant(const Matrix<T>& matrix);
```

正方行列の行列式を返します。

**制約:** 正方行列。係数Tは正確な体。

**計算量:** m行n列にO(m*n*min(m,n))時間・O(m*n)領域。

### MatrixInverse

```text
template<class T>std::optional<Matrix<T>> MatrixInverse(const Matrix<T>& matrix);
```

正方行列の逆行列をoptionalで返します。特異ならnulloptです。

**制約:** 正方行列。係数Tは正確な体。

**計算量:** O(n³)時間・O(n²)領域。

### SolveLinearSystem

```text
template<class T> std::optional<LinearSystemResult<T>> SolveLinearSystem(const Matrix<T>& matrix,const std::vector<T>& rhs);
```

連立一次方程式の一解と核の基底をoptionalで返します。

**制約:** rhs.size()==行数。係数Tは正確な体。

**計算量:** O(m*(n+1)*(1+min(m,n))+n²)時間・O(m*(n+1)+n²)領域。

### Matrix::Matrix

```text
Matrix();
Matrix(size_type rows,size_type columns,const T& value=T{0});
Matrix(size_type rows,size_type columns,std::vector<T> values);
Matrix(std::initializer_list<std::initializer_list<T>> rows);
Matrix(const Matrix&);
Matrix(Matrix&& other) noexcept;
```

空または指定した形・係数で行列を構築します。コピーは独立、移動元は0×0です。

**制約:** 行・列数と積が容量上限内。行リストは全行同長、flat vectorはr*c要素。

**計算量:** 既定・ムーブ・vector領域の移譲はO(1)、一様値・行リスト・コピーはO(r*c)。vector引数のコピーは別途必要。

### Matrix::operator=

```text
Matrix& operator=(Matrix other) noexcept;
```

コピーまたは移動で内容を置き換え、自身への参照を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** コピーO(r*c)、移動構築・swapはO(1)。代入は旧状態の破棄を加える。

### Matrix::swap

```text
void swap(Matrix& other) noexcept;
friend void swap(Matrix& a,Matrix& b) noexcept;
```

内容と所有する領域を交換します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### Matrix::rows

```text
size_type rows()const noexcept;
```

行数を返します。

**制約:** なし。

**計算量:** O(1)。

### Matrix::cols

```text
size_type cols()const noexcept;
```

列数を返します。

**制約:** なし。

**計算量:** O(1)。

### Matrix::size

```text
size_type size()const noexcept;
```

要素数を返します。

**制約:** なし。

**計算量:** O(1)。

### Matrix::empty

```text
bool empty()const noexcept;
```

空ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### Matrix::values

```text
std::span<T> values()noexcept;
std::span<const T> values()const noexcept;
```

全要素を参照するspanを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### Matrix::row

```text
std::span<T> row(size_type index);
std::span<const T> row(size_type index)const;
```

指定した行を参照するspanを返します。

**制約:** 行添字が範囲内。spanは再代入・swap・破棄までの借用。

**計算量:** O(1)。

### Matrix::operator()

```text
T& operator()(size_type row_index,size_type column_index);
const T& operator()(size_type row_index,size_type column_index)const;
```

指定した要素を返します。

**制約:** row_index<rows()、column_index<cols()。

**計算量:** O(1)。

### Matrix::assign

```text
void assign(size_type rows,size_type columns,const T& value=T{0});
```

指定した行数・列数・一様値で再初期化します。

**制約:** 行・列数と積が容量上限内。

**計算量:** O(1+r*c)時間・領域。

### Matrix::transposed

```text
Matrix transposed()const;
```

転置行列を返します。戻り値は元の行列から独立しています。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** r行c列にO(r*c)時間。独立な返却行列はO(r*c)領域。

### Matrix::operator+=

```text
Matrix& operator+=(const Matrix& other);
```

同形行列を加えて自身を更新し、自身への参照を返します。

**制約:** 両行列の行数・列数が同じ。

**計算量:** O(1+r*c)時間・O(1)追加領域。

### Matrix::operator-=

```text
Matrix& operator-=(const Matrix& other);
```

同形行列を引いて自身を更新し、自身への参照を返します。

**制約:** 両行列の行数・列数が同じ。

**計算量:** O(1+r*c)時間・O(1)追加領域。

### Matrix::operator*=

```text
Matrix& operator*=(const Matrix& other);
```

乗算して自身を更新します。

**制約:** 左行列の列数と右行列の行数が同じ。

**計算量:** (r,k)*(k,c)にO(r*c*(1+k))時間・O(r*c)追加領域。

### Matrix::operator+

```text
friend Matrix operator+(Matrix a,const Matrix& b);
```

同形行列の和を返します。

**制約:** 両行列の行数・列数が同じ。

**計算量:** O(1+r*c)時間・O(r*c)返却領域。

### Matrix::operator-

```text
friend Matrix operator-(Matrix a,const Matrix& b);
```

同形行列の差を返します。

**制約:** 両行列の行数・列数が同じ。

**計算量:** O(1+r*c)時間・O(r*c)返却領域。

### Matrix::operator==

```text
bool operator==(const Matrix&)const;
```

行数・列数・全要素が等しいかを返します。

**制約:** なし。

**計算量:** 最悪O(1+r*c)時間・O(1)追加領域。

### operator*

```text
template<class T> Matrix<T> operator*(const Matrix<T>& a,const Matrix<T>& b);
```

積を返します。

**制約:** 左行列の列数と右行列の行数が同じ。

**計算量:** (r,k)*(k,c)にO(r*c*(1+k))時間・O(r*c)追加領域。

### MatrixIdentity

```text
template<class T>Matrix<T> MatrixIdentity(std::size_t n);
```

単位行列を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(n²)時間・領域。

### MatrixPower

```text
template<class T>Matrix<T> MatrixPower(Matrix<T> base,std::uint64_t exponent);
```

正方行列の非負整数乗を返します。

**制約:** 正方行列・非負指数。

**計算量:** O(n³ log(k+2))時間・O(n²)領域。

### MatrixApply

```text
template<class T>std::vector<T> MatrixApply(const Matrix<T>& matrix,const std::vector<T>& vector);
```

行列と列ベクトルの積を返します。

**制約:** vector.size()==列数。

**計算量:** O(rows*(1+cols))時間・O(rows)領域。


次の例では[Fraction](../../Math/Fraction.cpp)も用意します。

```cpp
const sirkit::Matrix<sirkit::Fraction> a{{1,2}, {3,5}};
const auto rank = sirkit::MatrixRank(a);         // 2。
const auto det = sirkit::MatrixDeterminant(a);   // -1。
const auto inverse = sirkit::MatrixInverse(a);   // 正則なので値を持つ。
const auto result = sirkit::GaussianElimination(a, true);
// result.matrixは簡約化した行列、result.rank()は2。
```

消去の係数Tは正確な体である必要があります。素数法Mint・有理数は使えますが、通常の整数型・一般の合成数法・浮動小数点向けの消去ではありません。非可逆ピボットを検出する検査は、型全体が体であることの保証にはなりません。

`MatrixInverse`は非正則なら`std::nullopt`、`SolveLinearSystem(a,b)`は解がなければ`std::nullopt`を返します。解の`particular`は一解、`kernel`は各行が核の基底です。行列式は特異なら0、0×0では1です。消去は値で受け取るためlvalueの入力を変更しません。

係数演算をO(1)とすると、m行n列の消去・階数・行列式はO(m*n*min(m,n))時間・O(m*n)領域、n次行列の逆行列はO(n³)時間・O(n²)領域です。連立方程式はO(m*(n+1)*(1+min(m,n))+n²)時間・O(m*(n+1)+n²)領域で、核の出力も含みます。

[完全な使用例](../../Examples/gaussian_elimination_example.cpp)・[行列と消去の共通説明](../matrix.md)
