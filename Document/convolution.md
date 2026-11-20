# 任意法・整数畳み込み

[CRTConvolution](../Math/Convolution.cpp)は整数列a,bの畳み込み`c[k]=sum(a[i]*b[k-i])`を求めます。任意の32ビット正整数を法とする計算と、厳密なint64_tでの計算に対応します。Mintは不要です。入力は変更せず、片方が空なら空列を返します。[使用例](../Examples/convolution_example.cpp)を参照してください。

## API

### CRTConvolution::max_length

```text
static constexpr std::size_t max_length() noexcept;
```

対応する最大長を返します。

**制約:** なし。

**計算量:** O(1)。

### CRTConvolution::prepare

```text
void prepare(std::size_t length);
```

指定した範囲の計算用表を準備します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 新規準備O(length)時間・O(length)保持領域。

### CRTConvolution::roots_capacity

```text
std::size_t roots_capacity() const noexcept;
```

保持している根の個数を返します。

**制約:** なし。

**計算量:** O(1)。

### CRTConvolution::clear_cache

```text
void clear_cache() noexcept;
```

保持する計算用キャッシュを解放します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(保持する根の数)。

### CRTConvolution::modular

```text
template<std::integral T,std::integral U> std::vector<std::uint32_t> modular(const std::vector<T>& a,const std::vector<U>& b,std::uint32_t mod);
```

各入力を法で正規化し、畳み込みをvector<uint32_t>で返す。1<=mod<=UINT32_MAX。

**制約:** 結果長は2^23以下。整数版は全ての最終係数がint64_t内、任意法版は1<=mod<=UINT32_MAX。

**計算量:** 短辺32以下はO(nm)、他はL=bit_ceil(n+m-1)としてO(L log L)時間・O(L)領域。

### CRTConvolution::modular_square

```text
template<std::integral T> std::vector<std::uint32_t> modular_square(const std::vector<T>& a,std::uint32_t mod);
```

aとaの畳み込みを求めます。大きな入力では順変換を1回に減らします。

**制約:** 結果長は2^23以下。整数版は全ての最終係数がint64_t内、任意法版は1<=mod<=UINT32_MAX。

**計算量:** 短辺32以下はO(nm)、他はL=bit_ceil(n+m-1)としてO(L log L)時間・O(L)領域。

### CRTConvolution::integer

```text
std::vector<std::int64_t> integer(const std::vector<std::int64_t>& a,const std::vector<std::int64_t>& b);
```

int64_t列の畳み込みを厳密に求めます。

**制約:** 結果長は2^23以下。整数版は全ての最終係数がint64_t内、任意法版は1<=mod<=UINT32_MAX。

**計算量:** 短辺32以下はO(nm)、他はL=bit_ceil(n+m-1)としてO(L log L)時間・O(L)領域。

### CRTConvolution::integer_square

```text
std::vector<std::int64_t> integer_square(const std::vector<std::int64_t>& a);
```

自己畳み込みをvector<int64_t>で返す。

**制約:** 結果長は2^23以下。整数版は全ての最終係数がint64_t内、任意法版は1<=mod<=UINT32_MAX。

**計算量:** 短辺32以下はO(nm)、他はL=bit_ceil(n+m-1)としてO(L log L)時間・O(L)領域。

### ConvolutionAnyMod

```text
template<std::integral T,std::integral U> std::vector<std::uint32_t> ConvolutionAnyMod(const std::vector<T>& a,const std::vector<U>& b,std::uint32_t mod);
```

一時的にCRTConvolutionを作り、任意法の畳み込みを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 短辺32以下はO(nm)、他はL=bit_ceil(n+m-1)としてO(L log L)時間・O(L)領域。

### ConvolutionInt64

```text
std::vector<std::int64_t> ConvolutionInt64(const std::vector<std::int64_t>& a,const std::vector<std::int64_t>& b);
```

一時的にCRTConvolutionを作り、int64_tで厳密な畳み込みを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** 短辺32以下はO(nm)、他はL=bit_ceil(n+m-1)としてO(L log L)時間・O(L)領域。

### CRTConvolution::CRTConvolution

```text
CRTConvolution();
```

動的な根キャッシュを持たない状態で生成します。

**制約:** なし。

**計算量:** O(1)。

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

aとaの畳み込みを求めます。大きな入力では順変換を1回に減らします。

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

動的キャッシュを持たない状態で生成します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。


```cpp
CRTConvolution engine;
const std::vector<int> a{1,2}, b{3,4};
auto modular = engine.modular(a,b,7); // {3,3,1}。
const std::vector<std::int64_t> x{-1,2};
auto exact = engine.integer_square(x); // {1,-4,4}。
```

modularは符号付き・符号なし64ビット以下の整数入力を受け付け、INT64_MIN・UINT64_MAXも正規化できます。法1は全0、法0は常に例外です。**integerでは全ての最終係数がint64_tに収まることを利用側で保証します。** 中間積・部分和が大きくても、最終的に相殺される場合を扱います。

## 方式と計算量

3つの素数を法としてNTTを行い、中国剰余定理で結果を復元します。任意法版では正規化した係数と結果長の上限により一意性を確保します。指定法が同梱NTTの素数なら、そのNTTだけを使います。

短い方が32要素以下ならO(nm)、それ以外はL=bit_ceil(n+m−1)としてO(L log L)時間・O(L)追加領域です。結果長は2^23以下です。同じengineを再利用すると根の表も再利用でき、コピーの表は独立です。prepareの新規準備・保持量はO(length)、clear_cacheは保持数に比例します。

厳密整数版は、復元値がint64_tに収まらなければ検査省略時も例外です。ただし前提に違反する真の係数が3素数の積を超えると、小さな別の整数に合同となる場合があり、**値域違反を全て検出する保証はありません**。

GCC/Clangの128ビット整数拡張があれば厳密復元に使います。拡張型は標準C++の型ではありません。`SIRKIT_CONVOLUTION_DISABLE_INT128`または`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準の64ビット2語の代替経路を選べます。同梱NTTの追加SIMD・CPU判定は`SIRKIT_DISABLE_SIMD`で除外できます。容量・長さ・法0の保護は常に維持します。

Verifyは[Convolution (Mod 1,000,000,007)](https://judge.yosupo.jp/problem/convolution_mod_1000000007)と[AtCoder ABC392 G](https://atcoder.jp/contests/abc392/tasks/abc392_g)。数学的参考は[中国剰余定理](https://info.atcoder.jp/entry/algorithm_lectures/chinese_remainder_theorem)です。負数・相殺・極値・所有権は独立ローカル参照で補います。
