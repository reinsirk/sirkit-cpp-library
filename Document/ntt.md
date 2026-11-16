# NTTと素数法畳み込み

[NTT](../Math/NTT.cpp)は、素数を法として整数列の畳み込みを求めます。`NTT<>`の既定法は998244353・根3で、Mintは不要です。[使用例](../Examples/ntt_example.cpp)を参照してください。

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
NTT<> engine;
const std::vector<int> a{1,2}, b{3,4};
auto result = engine.convolution(a,b); // {3,10,8}。
std::vector<std::uint32_t> values{1,2,3,4};
engine.transform(values);
engine.transform(values,true); // 元のvaluesへ戻る。
```

畳み込みは片方が空なら空です。短い方が32要素以下ならO(nm)、それ以外はL=bit_ceil(n+m−1)としてO(L log L)時間・O(L)追加領域です。transformはO(n log n)時間、根以外の追加領域O(1)。prepareの新規準備・保持量はO(length)で、通常のコピーは根の表も独立に所有します。

## 法・入力と高速経路

`NTT<Mod,Root>`のModは3以上2^31未満の奇素数、Rootは最大の2冪位数を生成する値です。この条件はコンパイル時に確認します。変換上限は2^v2(Mod−1)で、既定法では2^23です。全乗法群を生成する根である必要はありません。

transformの長さは0または対応する2冪、各値は0以上Mod未満が前提です。長さ・容量の保護は`SKIP_RUNTIME_CHECKS`でも維持し、通常の値検査は演算処理へ組み込みます。transformは例外までの変更が残り得ます。

積には標準のuint64_tを使い、128ビット整数拡張は不要です。小さい法はMontgomery演算を使い、GCC/Clangの対応x86 CPUではAVX2専用経路も選びます。`SIRKIT_DISABLE_SIMD`で追加の専用命令とCPU判定を除外でき、標準経路も同じAPI・結果を保ちます。Verifyは[Library Checker](https://judge.yosupo.jp/problem/convolution_mod)です。
