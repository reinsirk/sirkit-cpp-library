# 64ビットの剰余演算・合同式・平方根

[本体](../Math/Modular_Arithmetic.cpp)は法1からUINT64_MAXまでの剰余計算を動的確保なしで行います。符号付きの値はNormalizeModで正規化して渡します。法1の剰余は0です。全APIはconstexprで、定数式の評価予算内ならコンパイル時にも使えます。[使用例](../Examples/modular_arithmetic_example.cpp)を参照してください。

## API

### NormalizeMod

```text
constexpr std::uint64_t NormalizeMod(std::int64_t value, std::uint64_t modulus);
```

符号付き整数を0以上modulus未満へ正規化します。

**制約:** modulus>=1。

**計算量:** O(1)。

### ModAdd

```text
constexpr std::uint64_t ModAdd(std::uint64_t a, std::uint64_t b, std::uint64_t modulus);
```

和の剰余を返します。

**制約:** modulus>=1。a,bは正規化不要。

**計算量:** O(1)。

### ModSubtract

```text
constexpr std::uint64_t ModSubtract(std::uint64_t a, std::uint64_t b, std::uint64_t modulus);
```

差の剰余を返します。

**制約:** modulus>=1。a,bは正規化不要。

**計算量:** O(1)。

### ModMultiply

```text
constexpr std::uint64_t ModMultiply(std::uint64_t a, std::uint64_t b, std::uint64_t modulus);
```

積の剰余を返します。

**制約:** modulus>=1。a,bは正規化不要。

**計算量:** 拡張整数経路O(1)、標準経路O(log(m+1))。

### ModPow

```text
constexpr std::uint64_t ModPow(std::uint64_t a, std::uint64_t exponent, std::uint64_t modulus);
```

非負整数乗の剰余を返します。

**制約:** modulus>=1、指数は非負。

**計算量:** 剰余乗算の費用をL(m)としてO(log(e+2)*L(m))時間・O(1)領域。

### ModInverse

```text
constexpr std::optional<std::uint64_t> ModInverse(std::uint64_t a, std::uint64_t modulus);
```

逆元をoptionalで返します。存在しなければnullopt、modulus=1では0です。

**制約:** modulus>=1。逆元がなければnullopt。

**計算量:** O(log(m+1)*L(m))時間・O(1)領域。合同式列は統合回数分を加える。

### Congruence::operator==

```text
friend constexpr bool operator==(const Congruence&, const Congruence&);
```

剰余と法が等しいかを返します。

**制約:** なし。

**計算量:** O(1)。

### SolveLinearCongruence

```text
constexpr std::optional<Congruence> SolveLinearCongruence(std::uint64_t a, std::uint64_t b, std::uint64_t modulus);
```

a*x=bの合同式の解をoptionalで返します。

**制約:** modulus>=1。

**計算量:** O(log(m+1)*L(m))時間・O(1)領域。合同式列は統合回数分を加える。

### ChineseRemainder

```text
constexpr std::optional<Congruence> ChineseRemainder(Congruence first, Congruence second);
constexpr std::optional<Congruence> ChineseRemainder(std::span<const Congruence> equations);
```

合同式を統合した解をoptionalで返します。解がなければnullopt、入力が空列なら{0,1}を返します。

**制約:** 法は正。解のある中間周期がuint64_tに収まる。

**計算量:** 統合1回にO(log(m+1)L(m))時間。k式なら合計O(k log(m+1)L(m))の上界、追加領域O(1)。

### ModSqrt

```text
constexpr std::optional<std::uint64_t> ModSqrt(std::uint64_t a, std::uint64_t prime);
```

素数法で平方根の小さい方をoptionalで返します。

**制約:** primeは素数。素数判定は行わない。

**計算量:** 非平方剰余をz候補調べる場合O((z+log p)log p)回の剰余乗算。


Congruenceは`{remainder,modulus}`で、解全体を`remainder + k*modulus`と表します。一次合同式ではmodulusはm/gcd(a,m)、中国剰余定理では元の法のLCMです。

```cpp
auto inverse = ModInverse(3, 11); // optionalに4。
auto equation = SolveLinearCongruence(4, 6, 10); // {4,5}: x=4 (mod 5)。
auto joined = ChineseRemainder({2,6}, {8,9}); // {8,18}。
auto root = ModSqrt(10, 13); // optionalに6。
```

## 前提と失敗

法0は常にinvalid_argumentです。CRTは先頭から式を統合し、解がないと分かった時点で終了します。後続の未参照入力までは検査しません。解のある途中の統合でLCMがUINT64_MAXを超えると、検査省略時もoverflow_errorです。後の式で解がなくなる場合も、この上限を超える中間周期は保持しません。

ModSqrtは2にも対応しますが、**素数判定はしません**。合成数を渡した場合の結果や終了は保証しません。`SKIP_RUNTIME_CHECKS`で素数候補の形の検査を除き、法0・不能判定・容量保護は維持します。全APIは入力不変です。

## 計算量と演算経路

剰余乗算1回の費用をL(m)とすると、累乗はO(log(e+2)*L(m))、逆元・一次合同式・CRTの1回の統合はO(log(m+1)*L(m))時間です。CRTでは統合法の最大値をmとして数えます。追加領域はいずれもO(1)です。NormalizeMod・ModAdd・ModSubtractはO(1)です。

GCC/Clangの128ビット整数拡張を使える経路ではL(m)=O(1)、標準の広い積ではL(m)=O(log(m+1))です。拡張型は標準C++の型ではありません。`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準整数の代替経路を選べます。大きい奇数法の累乗・平方根ではMontgomery表現を再利用します。

平方根は素数の形に応じた直接式、またはTonelli–Shanks法を使います。非平方剰余をz候補調べる場合はO((z+log p)*log p)回の剰余乗算で、小さいzを無条件の最悪上界にはしません。直接式の分岐はO(log p)回です。

Verifyは[Library Checker Sqrt Mod](https://judge.yosupo.jp/problem/sqrt_mod)、累乗・合同式はAtCoderの[Power](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_aq)・[Throne](https://atcoder.jp/contests/abc186/tasks/abc186_e)・[Oversleeping](https://atcoder.jp/contests/abc193/tasks/abc193_e)を使います。64ビット全域と容量超過は独立ローカル参照で補います。
