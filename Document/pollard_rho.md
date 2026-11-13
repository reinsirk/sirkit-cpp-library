# 64ビット整数の因数分解

`Math/Pollard_Rho.cpp`はuint64_t全域の正整数を、Brentの周期検出を使ったPollard Rho法で分解する。分解器`PollardRho64`は独立した擬似乱数状態だけを持つ。`prime_factors`は重複込み、`factorize`は素数と指数の組を昇順で返す。n=1の分解は空、n=0には有限の素因数分解がないため常に例外とする。

小さな素数を先に除き、7底の決定的Miller–Rabinで素数を確定する。合成数は平方数を先に処理し、それ以外をまとめたGCDと周期検出で分離する。1回の試行には計算量の上限を設け、行き詰まった場合は多項式を変えて再試行する。因数pを見つける期待計算量はO(sqrt(p))回の剰余乗算で、確定的な実行時間上界は保証しない。

剰余演算はMontgomery形式。GCC/Clangで128ビット整数拡張を使える場合も、還元の129ビット目を失わないよう上位和へ繰上がりを保存する。拡張がない場合は32ビットのlimbで同じ還元を行う。`SIRKIT_FORCE_STANDARD_ARITHMETIC`で後者を選べる。既存のMiller–Rabinファイルと共通の識別子・ガードを上書きしない。

`divisors`は約数のvectorを返し、整列を省くこともできる。`divisor_count`、`totient`、`mobius`も利用できる。φの計算は先に割ることで中間積のオーバーフローを避ける。約数の積は分解元のn以下で、容量の乗算は別途確認する。`PrimeFactors64`など末尾64の簡易関数は一時分解器を使う。

[使用例](../Examples/pollard_rho_example.cpp)、[Library Checker](https://judge.yosupo.jp/problem/factorize)を参照。
