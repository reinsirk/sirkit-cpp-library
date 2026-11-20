# 線形漸化式

[Berlekamp–Massey](../Math/Berlekamp_Massey.cpp)は有限列に合う最短の漸化式を求め、[Linear Recurrence](../Math/Linear_Recurrence.cpp)は係数と初期値から任意の項を求めます。後者は前者とNTTを同梱し、それぞれ単独で貼り付けられます。[復元の例](../Examples/berlekamp_massey_example.cpp)、[項・重みの例](../Examples/linear_recurrence_example.cpp)を参照してください。

dは漸化式の次数、kは0始まりの項番号、L=bit_ceil(2d+1)です。

## API

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

正規化済みuint32_tのvectorを値で受け取ります。moveで受け取った領域を再利用できます。

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

### BerlekampMassey

```text
template<class T> std::vector<T> BerlekampMassey(const std::vector<T>& sequence);
std::vector<std::uint32_t> BerlekampMassey(const std::vector<std::uint32_t>& sequence);
```

有限列に合う最短線形漸化式の係数を返します。

**制約:** 汎用Tは正確な体。uint32_t版は法998244353。

**計算量:** O(n²)時間・O(n)領域。

### LinearRecurrenceWeights

```text
template<class T> std::vector<T> LinearRecurrenceWeights(const std::vector<T>& coefficients,std::uint64_t k);
std::vector<std::uint32_t> LinearRecurrenceWeights(const std::vector<std::uint32_t>& coefficients,std::uint64_t k);
```

第k項を初期値の線形結合で表す重みを返します。

**制約:** 項番号は非負。汎用Tは正確な可換環。

**計算量:** O(d² log(k+2))時間・O(d)領域。k<dはO(d)。uint32_t版も同じ。

### LinearRecurrenceTerm

```text
template<class T> T LinearRecurrenceTerm(const std::vector<T>& initial,const std::vector<T>& coefficients,std::uint64_t k);
std::uint32_t LinearRecurrenceTerm(const std::vector<std::uint32_t>& initial, const std::vector<std::uint32_t>& coefficients,std::uint64_t k);
```

初期値と係数から0始まりの第k項を返します。

**制約:** 初期値数と係数数が等しく、項番号は非負。

**計算量:** 次数dに汎用版O(d² log(k+2))時間・O(d)領域。uint32_t版TermはO(L log L log(k+2))、L=bit_ceil(2d+1)。

### LinearRecurrencePrefix

```text
template<class T> std::vector<T> LinearRecurrencePrefix(const std::vector<T>& initial,const std::vector<T>& coefficients,std::size_t count);
std::vector<std::uint32_t> LinearRecurrencePrefix(const std::vector<std::uint32_t>& initial, const std::vector<std::uint32_t>& coefficients,std::size_t count);
```

先頭count項をvectorで返します。

**制約:** 初期値数と係数数が等しい。

**計算量:** O(count*(1+d))時間・O(count)領域。

### NTT::NTT

```text
NTT();
```

動的キャッシュを持たない状態で生成します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。


係数cがd個なら、`a[n]=c[0]*a[n-1]+...+c[d-1]*a[n-d]`です。初期値aもd個渡し、項の添字は0始まりです。次数0は初期値・係数とも空列で、全項0を表します。


```cpp
const std::vector<std::uint32_t> a{0,1}, c{1,1}; // Fibonacci数列。
auto term = LinearRecurrenceTerm(a,c,10); // 55。
auto prefix = LinearRecurrencePrefix(a,c,8); // {0,1,1,2,3,5,8,13}。
auto weights = LinearRecurrenceWeights(c,10); // {34,55}。
```

uint32_tのvector版は法998244353で計算し、入力を正規化します。Mintを用意せず使えます。汎用型Tを使う版ではMint等を利用側で用意してください。

## 型と復元の前提

BerlekampMasseyには正確な体の四則演算、0/1構築、等値比較が必要です。素数法Mintや有理数は使えますが、通常の整数型・一般の合成数法には使えません。浮動小数点誤差への対策もありません。

Term・Weights・Prefixの汎用版は除算を使わず、正確な可換環の加減乗算と0/1構築があれば使えます。合成数法Mintにも対応します。全APIは入力を変更しません。

**与えた有限列に合う漸化式でも、未知の後続項の値は保証できません。** 最短の漸化式の係数が一意に決まらない場合や、元の無限列の漸化式と異なる場合があります。末尾係数が0でも次数を縮めてよいとは限りません。例えば1,0,0の最短次数は1、係数は0です。

## 計算量と方式


係数演算をO(1)として数えます。汎用版は特性多項式による剰余環でx^kを二分累乗し、初期値への重みを求めます。uint32_t版TermはBostan–Mori法で求める添字を半分ずつにし、同梱NTTを使います。Weights・Prefixはuint32_t版も汎用版と同計算量です。

実際に使うNTT長は2^23以下です。対応CPUのAVX2経路は`SIRKIT_DISABLE_SIMD`で除外でき、標準C++の代替経路も使えます。長さ・容量の内部上限は検査省略時も維持します。共有部品は[同期生成器](../Scripts/generate_linear_recurrence.py)で2本の単独版へ埋め込みます。

Verifyは[Find Linear Recurrence](https://judge.yosupo.jp/problem/find_linear_recurrence)と[Kth term](https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence)。数学的参考は[復元講座](https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_reconstruction)・[第K項講座](https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_kth_term)、[Masseyの論文](https://www.isiweb.ee.ethz.ch/archive/massey_pub/pdf/BI411.pdf)・[Bostan–Moriの論文](https://specfun.inria.fr/bostan/BoMo20.pdf)、[Nyaanの復元](https://nyaannyaan.github.io/library/fps/berlekamp-massey.hpp)・[第N項](https://nyaannyaan.github.io/library/fps/nth-term.hpp)です。
