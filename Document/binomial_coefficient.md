# 二項係数

[整数版](../Math/Combination/Binomial_Mod.cpp)・[任意法Mint版](../Math/Combination/ModInt_Binomial.cpp)・[素数専用版](../Math/Combination/Prime_Binomial.cpp)・[使用例](../Examples/combination_integer_example.cpp)

`BinomialMod`は法`1..UINT32_MAX`の剰余を整数で返します。`ModIntBinomial<Mint>`は任意の法、`PrimeBinomial<Mint>`は素数の法でMintを返します。Mintは利用側で用意し、動的な法を変えたら計算器も作り直します。`n,k`は`std::int64_t`です。

## API

| 操作 | 動作 |
| --- | --- |
| `BinomialMod(modulus,max_n,options={})` | 法・前計算範囲・計算上限を指定 |
| `ModIntBinomial<Mint>(max_n,options={})` | Mintの法を使う任意法版 |
| `PrimeBinomial<Mint>(max_n,max_steps=1'000'000)` | 素数専用版。表外の積の回数上限も指定 |
| `prepare(max_n)` | 表を準備。各素数冪qについて最大`min(max_n,q-1)`まで |
| `COM(n,k)` | 二項係数。`n<0`、`k<0`、`k>n`は0 |
| `Lucas(n,k)` | 素数の法でCOMと同じ。任意法版の合成数・法1では例外 |
| `H(n,k)` | 重複組合せ。`0H0=1`、`0Hk=0`（k>0）。`n+k-1`の型上限超過は例外 |
| 整数版の`mod()` / `is_prime_modulus()` | 法 / 素数の法かをO(1)で返す |

構築と`prepare`の`max_n`は省略すると5,000,000です。法1では結果は0です。整数版には`BinomialMod(modulus,options)`もあります。

```cpp
BinomialOptions options;
options.table_limit = 200'000;
options.max_multiplicative_steps = 1'000'000;
BinomialMod comb(998244353, 200'000, options);
auto ways = comb.COM(10, 3); // 120
comb.prepare(200'000);
auto multi = comb.H(3, 2);  // 6
```

## 計算方法と計算量

素数pでは階乗・逆階乗表を使い、`n>=p`ではp進数の各桁の二項係数を掛けるLucasの定理を使います。合成数では各素数冪`p^e`で計算し、中国剰余定理で合成します。表外では積を使い、素数冪の場合は各項からpの因子を取り除きます。

以下は固定幅整数演算をO(1)とした上界です。mを法、rを異なる素因子数、Bを全表の総要素数とします。

| 処理 | 時間・領域 |
| --- | --- |
| 任意法版の構築 | O(sqrt(m)+B+r log m)時間。法の分解を含む |
| 任意法版の`prepare` | O(B+r log m)時間、全体O(B+r)領域 |
| 素数専用版の構築・`prepare` | O(B)回のMint演算と高々1回の逆元計算、O(B)領域 |
| 準備済みの素数法で`n<p` | O(1) |
| 全桁が表に収まるLucas | d桁としてO(d)、d=O(1+log_p(n+1)) |
| 素数法の表外Lucas | 積の総項数sとしてO(d+s)回の剰余演算と高々d回の逆元計算 |
| 準備済みの合成数法 | O(Σ_p(1+log_p(n+1)))。各素数因子の桁処理とCRTを含む |
| 素数冪p^eでの表外直接積 | s=min(k,n-k)としてO((s+1)(1+log_p(n+1))+log m) |

通常のmodintの逆元計算はO(log p)です。表の自動拡張には`prepare`の費用を加えます。`H`は対応する`COM`と同じ費用です。

## 上限と所有

`BinomialMod`と`ModIntBinomial`は表を自動拡張します。`PrimeBinomial`の表外照会は積で計算し、表を広げるときは明示的に`prepare`します。
`table_limit`は各表の最大添字で、既定値は5,100,000です。任意法版で表を持たず積だけを使う場合は、`max_n=0`と`table_limit=0`を指定します。`max_n=0`だけでは自動拡張は止まりません。
明示的な前計算が上限を超えた場合や、積の計算回数が上限を超えた場合は`length_error`です。`SKIP_RUNTIME_CHECKS`でも計算予算・容量保護・数学的な0判定を維持します。

`PrimeBinomial`はコピー・ムーブ構築が可能で、代入には対応しません。ムーブ元は`prepare`後に再利用できます。表の確保失敗では既存結果を保ちます。Mint演算は通常のmodintと同様に例外を投げないことを想定し、任意のMint演算が投げる場合の表更新の巻き戻しは保証しません。

理論参考：[A. Granville, Arithmetic properties of Binomial Coefficients](https://dms.umontreal.ca/~andrew/Binomial/intro.html)。
