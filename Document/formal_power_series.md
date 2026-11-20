# 形式的冪級数と多項式

[FormalPowerSeries](../Math/Formal_Power_Series.cpp)は、法998244353の多項式を扱います。係数は定数項から順に並べ、Mintは不要です。「精度n」の操作はx^n以上を捨て、n個の係数を返します。[使用例](../Examples/formal_power_series_example.cpp)を参照してください。

## API

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

係数列へのconst参照を返します。

**制約:** 返されたconst参照を使えるのは、所有者の更新・代入・破棄までです。

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

指定位置の係数を返します。範囲外では0を返します。

**制約:** iは任意。範囲外は0を返す。

**計算量:** O(1)。

### FormalPowerSeries::set

```text
template <std::integral T> void set(std::size_t i, T value);
```

指定した位置の係数を書き換えます。

**制約:** i<size()。

**計算量:** O(1)。

### FormalPowerSeries::resize

```text
void resize(std::size_t n);
```

係数列を指定した長さに切り詰めるか、足りない分を0で埋めます。

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

先頭n個の係数を持つコピーを返します。係数が足りない場合は0で埋めます。

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

微分した多項式を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 係数数nにO(n+1)時間。コピー・返却値はO(n)領域。

### FormalPowerSeries::integral

```text
FormalPowerSeries integral() const;
```

積分定数を0として積分した多項式を返します。積分前の係数数は法未満です。

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

非負整数乗を返します。

**制約:** 指数は非負。精度n<=2^22。n省略時は現在のsize()。

**計算量:** O(M(n)+log(k+1))時間・O(n)作業領域。

### FormalPowerSeries::sqrt

```text
std::optional<FormalPowerSeries> sqrt(std::size_t n) const;
std::optional<FormalPowerSeries> sqrt() const;
```

平方根をoptionalで返します。平方根が存在しなければnulloptを返します。

**制約:** 根が存在しなければnullopt。精度n<=2^22。n省略時は現在のsize()。

**計算量:** M(n)=O(n log(n+1))としてO(M(n))時間・O(n)作業領域。積・除算は対象長を用いる。

### FormalPowerSeries::divmod

```text
std::pair<FormalPowerSeries, FormalPowerSeries> divmod(const FormalPowerSeries& divisor) const;
```

多項式の商と剰余を組にして返します。零多項式による除算は常に例外になります。

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

動的キャッシュを作らずに構築します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。


```cpp
const FormalPowerSeries f{1,2,1}; // 1+2x+x²。
auto inverse = f.inverse(4); // {1,-2,3,-4}を法へ正規化した係数。
auto restored = f.log(4).exp(4); // f.prefix(4)と一致。
auto [quotient,remainder] = (f*f).divmod(f); // fと零多項式。
```

inverse・log・exp・pow・sqrtのnを省略すると、現在の係数数を精度にします。n=0は常に空列です。sqrtは先頭非0次数が偶数、かつ先頭係数が平方剰余なら存在します。先頭係数は2候補の小さい方を選び、精度内で未確定の末尾は0にします。powでは0^0=1です。

和・差は長い方の係数数、積は通常n+m−1個で、片方が空なら空です。商・剰余は末尾0を除いて返します。一方、通常の等値比較は係数数も比較するため、精度の異なる0列は区別します。

## 計算量・上限と所有

線形操作はO(n)、精度操作はM(n)=O(n log n)としてO(M(n))時間・O(n)作業領域です。powは加えてO(log k)がかかります。逆数はNewton反復で精度を倍にし、log・exp・平方根も畳み込みを組み合わせて求めます。

inverse・log・exp・pow・sqrtの精度は2^22以下、積・商剰余のNTT長は2^23以下です。容量・長さ・数学的な0判定は検査省略時も維持します。

係数は各個体が独立に所有します。NTT根と整数逆数の表だけを同一スレッドで再利用し、別スレッドとは共有しません。[同梱NTT](ntt.md)は共通ガードで単独版との併用もでき、[生成器](../Scripts/generate_formal_power_series.py)で同期します。VerifyはLibrary Checkerの形式的冪級数・多項式除算を使います。
