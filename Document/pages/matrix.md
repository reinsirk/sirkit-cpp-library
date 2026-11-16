# 行列の演算と累乗

`Matrix<T>`は、r行c列の要素を連続配列として所有します。行列の加減・積・転置、列ベクトルとの積、正方行列の非負整数乗を扱います。

Matrixの`size_type`は`std::size_t`、`value_type`はTです。

行列のr・cは行数・列数、kは積の中間次元、nは正方行列の次数です。係数演算・コピーをO(1)として数えます。

## API

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

要素の参照。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### Matrix::row

```text
std::span<T> row(size_type index);
std::span<const T> row(size_type index)const;
```

要素の参照。

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

転置の独立コピー。

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


```cpp
const sirkit::Matrix<std::int64_t> a{{1,2}, {3,4}};
const auto product = a * a; // {{7,10}, {15,22}}。
const auto x = sirkit::MatrixApply(a, std::vector<std::int64_t>{1,2}); // {5,11}。
const auto identity = sirkit::MatrixPower(a, 0); // 2次単位行列。
```

Tはコピー可能で、0/1からの構築と使う操作に必要な環の演算を提供します。整数も使えますが、中間値のオーバーフローや浮動小数点誤差は利用側で管理します。`bool`用の行列ではありません。

係数演算・コピーをO(1)とすると、加減・転置はO(r*c)、行列積(r,k)*(k,c)はO(r*c*(1+k))時間・O(r*c)追加領域です。n次正方行列のk乗はO(n³ log(k+2))時間・O(n²)追加領域、列ベクトルとの積はO(r*(1+c))時間・O(r)領域です。

`a(r,c)`は要素への参照、`row(r)`・`values()`は非所有のspanです。行列の代入・swap・破棄をまたいで保持しません。コピーは独立し、move元は0×0です。0行・0列も形を保ちます。

行列式・階数・逆行列・連立一次方程式は、このファイルのAPIには含みません。必要な場合はMatrixも同梱する[消去ライブラリ](../../Math/Gaussian_Elimination.cpp)を使います。

[完全な使用例](../../Examples/matrix_example.cpp)・[行列の共通説明](../matrix.md)
