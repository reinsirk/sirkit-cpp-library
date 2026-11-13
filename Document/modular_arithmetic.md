# 64ビットの剰余演算・合同式・平方根

[本体](../Math/Modular_Arithmetic.cpp)は、法1からUINT64_MAXまでの剰余計算を動的確保なしで行います。符号付きの値はNormalizeModで正規化して渡します。法1の剰余は0です。全APIはconstexprで、定数式の評価予算内ならコンパイル時にも使えます。[使用例](../Examples/modular_arithmetic_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `NormalizeMod(int64_t a,uint64_t m)` | 負数を含むaを0以上m未満へ正規化する。INT64_MINも可。 |
| `ModAdd(a,b,m)` / `ModSubtract(a,b,m)` / `ModMultiply(a,b,m)` | uint64_tの和 / 差 / 積の剰余を返す。a,bは正規化不要。 |
| `ModPow(a,e,m)` | a^eの剰余。指数0は1 mod m。 |
| `ModInverse(a,m)` | 逆元のoptional。存在しなければnullopt、m=1では0。合成数法も可。 |
| `SolveLinearCongruence(a,b,m)` | a*x=b (mod m)のoptional<Congruence>。不能ならnullopt。 |
| `ChineseRemainder(a,b)` / `(span<const Congruence>)` | 合同式を統合する。互いに素でない法も可。不能はnullopt、空列は{0,1}。 |
| `ModSqrt(a,p)` | x²=a (mod p)の根の小さい方をoptionalで返す。根がなければnullopt。pは素数が前提。 |

Congruenceは`{remainder,modulus}`で、解全体を`remainder + k*modulus`と表します。一次合同式ではmodulusはm/gcd(a,m)、中国剰余定理では元の法のLCMです。

```cpp
auto inverse = ModInverse(3, 11); // optionalに4。
auto equation = SolveLinearCongruence(4, 6, 10); // {4,5}: x=4 (mod 5)。
auto joined = ChineseRemainder({2,6}, {8,9}); // {8,18}。
auto root = ModSqrt(10, 13); // optionalに6。
```

## 前提と失敗

法0は常にinvalid_argumentです。CRTは先頭から統合し、不能ならその場で終了するため、後続の未参照入力までは検査しません。解のある途中の統合でLCMがUINT64_MAXを超えると、検査省略時もoverflow_errorです。後の式で不能になる場合も、この上限を超える中間周期は保持しません。

ModSqrtは2にも対応しますが、**素数判定はしません**。合成数を渡した場合の結果や終了は保証しません。`SKIP_RUNTIME_CHECKS`で素数候補の形の検査を除き、法0・不能判定・容量保護は維持します。全APIは入力不変です。

## 計算量と演算経路

剰余乗算1回の費用をL(m)とすると、累乗はO(log(e+2)*L(m))、逆元・一次合同式・CRTの1回の統合はO(log(m+1)*L(m))時間です。CRTでは統合法の最大値をmとして数えます。追加領域はいずれもO(1)です。NormalizeMod・ModAdd・ModSubtractはO(1)です。

GCC/Clangの128ビット整数拡張を使える経路ではL(m)=O(1)、標準の広い積ではL(m)=O(log(m+1))です。拡張型は標準C++の型ではありません。`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準整数の代替経路を選べます。大きい奇数法の累乗・平方根ではMontgomery表現を再利用します。

平方根は素数の形に応じた直接式、またはTonelli–Shanks法を使います。非平方剰余をz候補調べる場合はO((z+log p)*log p)回の剰余乗算で、小さいzを無条件の最悪上界にはしません。直接式の分岐はO(log p)回です。

Verifyは[Library Checker Sqrt Mod](https://judge.yosupo.jp/problem/sqrt_mod)、累乗・合同式はAtCoderの[Power](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_aq)・[Throne](https://atcoder.jp/contests/abc186/tasks/abc186_e)・[Oversleeping](https://atcoder.jp/contests/abc193/tasks/abc193_e)を使います。64ビット全域と容量超過は独立ローカル参照で補います。
