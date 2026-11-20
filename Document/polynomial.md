# 多項式の多点評価・補間・Taylor shift

[ProductTreeと多項式操作](../Math/Polynomial.cpp)は、法998244353で多数の点の評価・値からの補間・f(X+c)への変換を行います。Mintは不要です。同梱の[FormalPowerSeries](formal_power_series.md)・[NTT](ntt.md)とは共通ガードを使い、同時に貼り付けられます。[使用例](../Examples/polynomial_example.cpp)を参照してください。

## API

### ProductTree::max_points

```text
static constexpr std::size_t max_points() noexcept;
```

保持できる最大点数を返します。

**制約:** なし。

**計算量:** O(1)。

### ProductTree::ProductTree

```text
ProductTree();
template<std::integral T>explicit ProductTree(const std::vector<T>& points);
ProductTree(const ProductTree&);
ProductTree(ProductTree&& other) noexcept;
```

各点の因子(X−x_i)をまとめた積木を構築します。評価を繰り返す場合は同じ木を再利用できます。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 点列からの構築O(1+M(n)log(n+1))、コピーO(1+n log(n+1))、既定・移動構築O(1)。

### ProductTree::operator=

```text
ProductTree& operator=(const ProductTree& other);
ProductTree& operator=(ProductTree&& other) noexcept;
```

コピーまたは移動で内容を置き換え、自身への参照を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** コピーは新旧の保持量に比例、移動は旧状態の解放を含む。

### ProductTree::swap

```text
void swap(ProductTree& other) noexcept;
friend void swap(ProductTree& a,ProductTree& b) noexcept;
```

内容と所有する領域を交換します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### ProductTree::assign

```text
template<std::integral T>void assign(const std::vector<T>& points);
```

各点の因子(X−x_i)をまとめた積木を構築します。評価を繰り返す場合は同じ木を再利用できます。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** M(k)=O(k log(k+1))としてO(1+M(n)log(n+1))時間・O(n log(n+1))領域。コピー・移動は保持量に従う。

### ProductTree::size

```text
std::size_t size()const noexcept;
```

要素数を返します。

**制約:** なし。

**計算量:** O(1)。

### ProductTree::empty

```text
bool empty()const noexcept;
```

空ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### ProductTree::points

```text
const std::vector<std::uint32_t>& points()const noexcept;
```

正規化済みの点列へのconst参照を返します。

**制約:** 返したconst参照は所有者の更新・代入・破棄までの借用。

**計算量:** O(1)。

### ProductTree::polynomial

```text
FormalPowerSeries polynomial()const;
```

全点の積Π(X−x_i)をFormalPowerSeriesで返します。点列が空なら定数1を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(n+1)時間・返却領域。

### ProductTree::evaluate

```text
std::vector<std::uint32_t> evaluate(const FormalPowerSeries& f)const;
template<std::integral T>std::vector<std::uint32_t> evaluate(const std::vector<T>& coefficients)const;
```

各点で評価した値を、入力点と同じ順にvector<uint32_t>で返します。同じ点が複数あっても構いません。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 点数n・係数数dにO(1+M(max(n,d))log(n+2)+n+d)時間・O(n+d)追加領域。簡易版は構築も加える。

### ProductTree::interpolate

```text
template<std::integral T>FormalPowerSeries interpolate(const std::vector<T>& values)const;
```

点と値から次数n未満のFormalPowerSeriesを返します。点は法の下で互いに異なり、値の個数は点の個数と一致する必要があります。

**制約:** 点は法998244353で相異なり、値の個数は点数と同じ。

**計算量:** M(k)=O(k log(k+1))としてO(1+M(n)log(n+1))時間・O(n log(n+1))領域。コピー・移動は保持量に従う。

### MultipointEvaluate

```text
template<std::integral T> std::vector<std::uint32_t> MultipointEvaluate(const FormalPowerSeries& f,const std::vector<T>& points);
template<std::integral T,std::integral U> std::vector<std::uint32_t> MultipointEvaluate(const std::vector<T>& f,const std::vector<U>& points);
```

一時積木を使う簡易版。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 木の構築と評価にO(1+M(max(n,d))log(n+2)+n+d)時間、総O(n log(n+1)+d+1)領域。

### PolynomialInterpolate

```text
template<std::integral T,std::integral U> FormalPowerSeries PolynomialInterpolate(const std::vector<T>& points,const std::vector<U>& values);
```

一時積木を使う簡易版。

**制約:** 点は法998244353で相異なり、値の個数は点数と同じ。

**計算量:** M(k)=O(k log(k+1))としてO(1+M(n)log(n+1))時間・O(n log(n+1))領域。コピー・移動は保持量に従う。

### TaylorShift

```text
template<std::integral T> FormalPowerSeries TaylorShift(const FormalPowerSeries& f,T shift);
template<std::integral T,std::integral U> FormalPowerSeries TaylorShift(const std::vector<T>& f,U shift);
```

f(X+c)のFormalPowerSeriesを、入力と同じ係数数で返す。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** M(d)=O(d log(d+1))としてO(1+M(d))時間・O(d)領域。

### FormalPowerSeries::modulus

```text
static constexpr std::uint32_t modulus() noexcept;
```

使用する法を返します。

**制約:** なし。

**計算量:** O(1)。

### FormalPowerSeries::max_precision

```text
static constexpr std::size_t max_precision() noexcept;
```

精度操作の最大係数数を返します。

**制約:** なし。

**計算量:** O(1)。

### FormalPowerSeries::FormalPowerSeries

```text
FormalPowerSeries();
explicit FormalPowerSeries(std::size_t n);
explicit FormalPowerSeries(std::vector<std::uint32_t> values);
template <std::integral T> explicit FormalPowerSeries(const std::vector<T>& values);
FormalPowerSeries(std::initializer_list<std::int64_t> values);
```

空・指定係数数の零列・正規化した整数係数列から構築します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::size

```text
std::size_t size() const noexcept;
```

要素数を返します。

**制約:** なし。

**計算量:** O(1)。

### FormalPowerSeries::empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### FormalPowerSeries::coefficients

```text
const std::vector<std::uint32_t>& coefficients() const noexcept;
```

const参照。

**制約:** 返したconst参照は所有者の更新・代入・破棄までの借用。

**計算量:** O(1)。

### FormalPowerSeries::operator[]

```text
std::uint32_t operator[](std::size_t i) const;
```

指定した要素を返します。

**制約:** i<size()。

**計算量:** O(1)。

### FormalPowerSeries::get

```text
std::uint32_t get(std::size_t i) const noexcept;
```

指定した係数を返します。範囲外なら0を返します。

**制約:** iは任意。範囲外は0を返す。

**計算量:** O(1)。

### FormalPowerSeries::set

```text
template <std::integral T> void set(std::size_t i, T value);
```

既存の係数を変更します。

**制約:** i<size()。

**計算量:** O(1)。

### FormalPowerSeries::resize

```text
void resize(std::size_t n);
```

長さが足りなければ0を補い、長ければ切り詰めます。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::trim

```text
void trim() noexcept;
```

末尾の零係数を除きます。零多項式は空になります。

**制約:** なし。

**計算量:** O(n+1)時間・O(1)追加領域。

### FormalPowerSeries::prefix

```text
FormalPowerSeries prefix(std::size_t n) const;
```

n個の係数のコピーを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::operator+=

```text
FormalPowerSeries& operator+=(const FormalPowerSeries& other);
```

加算して自身を更新します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::operator-=

```text
FormalPowerSeries& operator-=(const FormalPowerSeries& other);
```

減算して自身を更新します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::operator*=

```text
FormalPowerSeries& operator*=(const FormalPowerSeries& other);
```

乗算して自身を更新します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::operator/=

```text
FormalPowerSeries& operator/=(const FormalPowerSeries& other);
```

除算して自身を更新します。

**制約:** 除数は零多項式でないこと。必要なNTT長は2^23以下。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::operator%=

```text
FormalPowerSeries& operator%=(const FormalPowerSeries& other);
```

多項式の剰余で自身を更新します。

**制約:** 除数は零多項式でないこと。必要なNTT長は2^23以下。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::operator+

```text
friend FormalPowerSeries operator+(FormalPowerSeries a, const FormalPowerSeries& b);
```

係数ごとの和を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::operator-

```text
friend FormalPowerSeries operator-(FormalPowerSeries a, const FormalPowerSeries& b);
```

係数ごとの差を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::operator*

```text
friend FormalPowerSeries operator*(const FormalPowerSeries& a, const FormalPowerSeries& b);
```

積を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::operator/

```text
friend FormalPowerSeries operator/(const FormalPowerSeries& a, const FormalPowerSeries& b);
```

商を返します。

**制約:** 除数は零多項式でないこと。必要なNTT長は2^23以下。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::operator%

```text
friend FormalPowerSeries operator%(const FormalPowerSeries& a, const FormalPowerSeries& b);
```

多項式の剰余を返します。

**制約:** 除数は零多項式でないこと。必要なNTT長は2^23以下。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::operator==

```text
friend bool operator==(const FormalPowerSeries&, const FormalPowerSeries&);
```

長さと係数列が完全に一致するかを返します。

**制約:** なし。

**計算量:** 最悪O(n+1)時間・O(1)追加領域。

### FormalPowerSeries::scale

```text
template <std::integral T> FormalPowerSeries scale(T scalar) const;
```

全係数をscalar倍したコピーを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::derivative

```text
FormalPowerSeries derivative() const;
```

微分した結果を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::integral

```text
FormalPowerSeries integral() const;
```

積分。積分定数0、積分前の係数数は法未満。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::inverse

```text
FormalPowerSeries inverse(std::size_t n) const;
FormalPowerSeries inverse() const;
```

x^n未満で形式的冪級数の逆数を返します。

**制約:** n>0のとき定数項は非零。n<=2^22。n=0は空列、n省略時はsize()。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::log

```text
FormalPowerSeries log(std::size_t n) const;
FormalPowerSeries log() const;
```

x^n未満で形式的対数を返します。

**制約:** n>0のとき定数項は1。n<=2^22。n=0は空列、n省略時はsize()。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::exp

```text
FormalPowerSeries exp(std::size_t n) const;
FormalPowerSeries exp() const;
```

x^n未満で形式的指数を返します。

**制約:** n>0のとき定数項は0。n<=2^22。n=0は空列、n省略時はsize()。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::pow

```text
FormalPowerSeries pow(std::uint64_t exponent, std::size_t n) const;
FormalPowerSeries pow(std::uint64_t exponent) const;
```

非負整数乗。

**制約:** 指数は非負。精度n<=2^22。n省略時は現在のsize()。

**計算量:** O(M(n)+log(k+1))時間・O(n)作業領域。

### FormalPowerSeries::sqrt

```text
std::optional<FormalPowerSeries> sqrt(std::size_t n) const;
std::optional<FormalPowerSeries> sqrt() const;
```

平方根をoptionalで返します。平方根が存在しなければnulloptです。

**制約:** 根が存在しなければnullopt。精度n<=2^22。n省略時は現在のsize()。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::divmod

```text
std::pair<FormalPowerSeries, FormalPowerSeries> divmod(const FormalPowerSeries& divisor) const;
```

多項式の商と剰余の組 / 商 / 剰余。0除算は常に例外。

**制約:** 除数は零多項式でないこと。必要なNTT長は2^23以下。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::clear_cache

```text
static void clear_cache() noexcept;
```

保持する計算用キャッシュを解放します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 保持キャッシュの解放費用。

### NTT::modulus

```text
static constexpr std::uint32_t modulus() noexcept;
```

使用する法を返します。

**制約:** なし。

**計算量:** O(1)。

### NTT::max_length

```text
static constexpr std::size_t max_length() noexcept;
```

対応する最大長を返します。

**制約:** なし。

**計算量:** O(1)。

### NTT::roots_capacity

```text
std::size_t roots_capacity() const noexcept;
```

保持している根の個数を返します。

**制約:** なし。

**計算量:** O(1)。

### NTT::clear_cache

```text
void clear_cache() noexcept;
```

保持する計算用キャッシュを解放します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(保持する根の数)。

### NTT::prepare

```text
void prepare(std::size_t length);
```

指定した範囲の計算用表を準備します。

**制約:** lengthは0またはmax_length()以下の2冪。

**計算量:** 新規準備O(length)時間・保持O(length)領域。

### NTT::transform

```text
void transform(std::vector<std::uint32_t>& a, bool inverse = false);
```

係数と周波数を正順で相互変換する。aを直接変更する。

**制約:** 長さは0または対応する2冪、各係数は0..Mod-1。

**計算量:** O(n log(n+1))時間、根以外の追加領域O(1)。

### NTT::convolution

```text
template <std::integral T, std::integral U> std::vector<std::uint32_t> convolution(const std::vector<T>& a, const std::vector<U>& b);
```

整数vectorを法へ正規化し、vector<uint32_t>の畳み込みを返す。負数・UINT64_MAXも可。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 短辺32以下はO(nm)、他はL=bit_ceil(n+m-1)としてO(L log L)時間・O(L)領域。

### NTT::convolution_normalized

```text
std::vector<std::uint32_t> convolution_normalized(std::vector<std::uint32_t> a, std::vector<std::uint32_t> b);
```

正規化済みuint32_tのvectorを値で受け取る。moveした領域を再利用可。

**制約:** 各係数は0..Mod-1。必要なNTT長は上限以下。

**計算量:** 短辺32以下はO(nm)、他はO(L log L)時間・O(L)領域。

### NTT::square

```text
template <std::integral T> std::vector<std::uint32_t> square(const std::vector<T>& a);
```

aとaの畳み込み。大きい入力では順変換を1回にする。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 短い入力はO(n²)、他はL=bit_ceil(2n-1)としてO(L log L)時間・O(L)領域。

### ConvolutionMod

```text
template <std::uint32_t Mod = 998244353, std::uint32_t Root = 3, std::integral T, std::integral U> std::vector<std::uint32_t> ConvolutionMod(const std::vector<T>& a, const std::vector<U>& b);
```

一時NTTを使い素数法の畳み込みを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** NTT::convolutionと同じ。

### NTT::NTT

```text
NTT();
```

動的キャッシュなしで生成。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。


```cpp
const FormalPowerSeries f{1,2,3}; // 1+2X+3X²。
ProductTree tree(std::vector<int>{0,1,2});
auto values = tree.evaluate(f); // {1,6,17}。
auto restored = tree.interpolate(values); // fと一致。
auto shifted = TaylorShift(f,1); // {6,8,3}。
```

polyはFormalPowerSeriesまたは整数係数vectorです。点・係数・値は符号付き／符号なし64ビット以下の整数を法へ正規化し、INT64_MIN・UINT64_MAXも扱えます。補間結果はn係数を保持し、末尾0も残します。空の点列からの評価・補間は空です。

## 前提と計算量

点数nは2^22−1以下、係数数dは2^22以下です。補間の重複点は数式上の分母0となるため、`SKIP_RUNTIME_CHECKS`でもdomain_errorです。値列の長さ検査は省略できますが、容量・数学的な0判定は維持します。

M(k)=O(k log(k+1))とします。積木の構築はO(1+M(n)log(n+1))時間・O(n log(n+1))保持。評価はO(1+M(max(n,d))log(n+2)+n+d)時間・O(n+d)追加領域です。補間はO(1+M(n)log(n+1))時間・O(n log(n+1))追加領域、TaylorShiftはO(1+M(d))時間・O(d)追加領域です。

評価は子の積による剰余を繰り返して葉の値を求めます。補間は積の微分から分母を作って各点の寄与を結合し、TaylorShiftは階乗を使う畳み込みで求めます。内部手順を利用側で呼び分ける必要はありません。

ProductTreeは点と積を独立に所有します。コピーは独立、別個体へのmove元は空で再利用可です。assignとコピー代入は失敗時に元を保ち、評価・補間は木を変更しません。同梱FPSの根cacheは同一スレッドで共有し、必要なら`FormalPowerSeries::clear_cache()`で解放します。追加SIMD・CPU判定は`SIRKIT_DISABLE_SIMD`で除外できます。

数式・用途の参考は[Nyaanの多点評価](https://nyaannyaan.github.io/library/fps/multipoint-evaluation.hpp.html)、Verifyは[Polynomial Interpolation](https://judge.yosupo.jp/problem/polynomial_interpolation)などです。積木・評価・補間の実装は独立に構成し、NTT/FPSは既存の自作CC0部品を同期しています。
