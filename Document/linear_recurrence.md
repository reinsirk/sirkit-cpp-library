# 線形漸化式

[Berlekamp–Massey](../Math/Berlekamp_Massey.cpp)は有限列に合う最短の漸化式を求め、[Linear Recurrence](../Math/Linear_Recurrence.cpp)は係数と初期値から任意の項を求めます。後者は前者とNTTを同梱し、それぞれ単独で貼り付けられます。[復元の例](../Examples/berlekamp_massey_example.cpp)、[項・重みの例](../Examples/linear_recurrence_example.cpp)を参照してください。

係数cがd個なら、`a[n]=c[0]*a[n-1]+...+c[d-1]*a[n-d]`です。初期値aもd個渡し、項の添字は0始まりです。次数0は初期値・係数とも空列で、全項0を表します。

## API

| API | 動作 |
| --- | --- |
| `BerlekampMassey(s)` | 有限列sの全体に合う最短次数の係数vectorを返す。空列・全零列は空。 |
| `LinearRecurrenceTerm(a,c,k)` | 0始まりの第k項を返す。 |
| `LinearRecurrenceWeights(c,k)` | a[k]を初期d項の線形結合で表す、長さdの重みを返す。 |
| `LinearRecurrencePrefix(a,c,count)` | 先頭count項をvectorで返す。 |

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

**有限列への適合だけでは、未知の後続項は保証できません。** 最短の係数が一意でない場合や、元の無限列の漸化式と異なる場合があります。末尾係数が0でも次数を縮めてよいとは限りません。例えば1,0,0の最短次数は1、係数は0です。

## 計算量と方式

| 操作 | 計算量・領域 |
| --- | --- |
| BerlekampMassey | O(n²)時間・O(n)領域。 |
| 汎用版Term / Weights | O(d² log(k+2))時間・O(d)領域。Weightsのk<dはO(d)。 |
| Prefix | O(count*(1+d))時間・O(count)領域。 |
| uint32_t版Term | L=bit_ceil(2d+1)としてO(L log L log(k+2))時間・O(L)領域。 |

係数演算をO(1)として数えます。汎用版は特性多項式による剰余環でx^kを二分累乗し、初期値への重みを求めます。uint32_t版TermはBostan–Mori法で求める添字を半分ずつにし、同梱NTTを使います。Weights・Prefixはuint32_t版も汎用版と同計算量です。

実際に使うNTT長は2^23以下です。対応CPUのAVX2経路は`SIRKIT_DISABLE_SIMD`で除外でき、標準C++の代替経路も使えます。長さ・容量の内部上限は検査省略時も維持します。共有部品は[同期生成器](../Scripts/generate_linear_recurrence.py)で2本の単独版へ埋め込みます。

Verifyは[Find Linear Recurrence](https://judge.yosupo.jp/problem/find_linear_recurrence)と[Kth term](https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence)。数学的参考は[復元講座](https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_reconstruction)・[第K項講座](https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_kth_term)、[Masseyの論文](https://www.isiweb.ee.ethz.ch/archive/massey_pub/pdf/BI411.pdf)・[Bostan–Moriの論文](https://specfun.inria.fr/bostan/BoMo20.pdf)、[Nyaanの復元](https://nyaannyaan.github.io/library/fps/berlekamp-massey.hpp)・[第N項](https://nyaannyaan.github.io/library/fps/nth-term.hpp)です。実装・測定の記録は[Audit](../Audit/)に置きます。
