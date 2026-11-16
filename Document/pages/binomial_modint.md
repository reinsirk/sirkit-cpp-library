# 二項係数（整数・Mint、任意法）

`ModIntBinomial<Mint>`は、利用側が用意するAtCoder modint互換型の法で二項係数を求め、Mintで返します。素数・合成数の法に対応します。整数を返す`BinomialMod`と共通設定の`BinomialOptions`も同梱しています。

BinomialModの`index_type`は`std::int64_t`、`value_type`は`std::uint32_t`です。

計算量のmは法、rは異なる素因子数、Bは全表の総要素数、dはLucasの桁数、sは表外の積の総項数です。通常のMintの逆元計算はO(log m)です。

## API

### ModIntBinomial::ModIntBinomial

```text
explicit ModIntBinomial(std::int64_t max_n = 5'000'000, BinomialOptions options = {});
explicit ModIntBinomial(BinomialOptions options);
```

Mint::mod()を法として構築・前計算。

**制約:** max_n>=0。前計算範囲はBinomialOptionsの上限以内。動的Mintの法は構築前に設定し、変更後は再構築。

**計算量:** 構築はO(sqrt(m)+B+r log m)時間・O(B+r)領域。

### ModIntBinomial::prepare

```text
void prepare(std::int64_t max_n = 5'000'000);
```

指定した範囲の計算用表を準備します。

**制約:** 0<=max_nでBinomialOptionsの表上限以内。

**計算量:** O(B+r log m)時間・O(B+r)領域。

### ModIntBinomial::COM

```text
Mint COM(std::int64_t n, std::int64_t k);
```

二項係数nCkを法の剰余で返します。

**制約:** n,kはint64_t。n<0・k<0・k>nは0。動的Mintの法は途中で変更しない。

**計算量:** 素数法でn<法かつ準備済みならO(1)。Lucasはd桁と表外の積s項にO(d+s)回のMint演算と高々d回の逆元計算。任意法版は整数版に従う。

### ModIntBinomial::Lucas

```text
Mint Lucas(std::int64_t n, std::int64_t k);
```

素数法でLucasの定理によるnCkを返します。

**制約:** 法は素数。合成数・法1にはCOMを使う。

**計算量:** d桁の表内はO(d)。表外は積の総項数sとしてO(d+s)回の係数演算と高々d回の逆元計算。

### ModIntBinomial::H

```text
Mint H(std::int64_t n, std::int64_t k);
```

重複組合せnHkを返します。

**制約:** 負のnまたはkでは0。0H0=1、0Hk=0（k>0）。通常のCOMへ渡すn+k-1はint64_t内。法1なら0。

**計算量:** N=n+k-1としたCOM(N,k)と同じ。素数法でN<法かつ準備済みならO(1)、表外と自動拡張は対応する費用。

### BinomialMod::BinomialMod

```text
explicit BinomialMod(index_type modulus = 998244353, index_type max_n = 5'000'000, BinomialOptions options = {});
BinomialMod(index_type modulus, BinomialOptions options);
```

法・前計算範囲・上限を指定。

**制約:** max_n>=0。前計算範囲はBinomialOptionsの上限以内。1<=modulus<=UINT32_MAX。

**計算量:** 構築はO(sqrt(m)+B+r log m)時間・O(B+r)領域。

### BinomialMod::mod

```text
value_type mod() const noexcept;
```

法を返します。

**制約:** なし。

**計算量:** O(1)。

### BinomialMod::is_prime_modulus

```text
bool is_prime_modulus() const noexcept;
```

法が素数ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### BinomialMod::prepare

```text
void prepare(index_type max_n = 5'000'000);
```

指定した範囲の計算用表を準備します。

**制約:** 0<=max_nでBinomialOptionsの表上限以内。

**計算量:** O(B+r log m)時間・O(B+r)領域。

### BinomialMod::COM

```text
value_type COM(index_type n, index_type k);
```

二項係数nCkを法の剰余で返します。

**制約:** n,kはint64_t。n<0・k<0・k>nは0。

**計算量:** 素数法でn<法かつ準備済みならO(1)。一般法はO(Σ_p(1+log_p(n+1)))、表外直接積・自動拡張の費用を加える。

### BinomialMod::Lucas

```text
value_type Lucas(index_type n, index_type k);
```

素数法でLucasの定理によるnCkを返します。

**制約:** 法は素数。合成数・法1にはCOMを使う。

**計算量:** d桁の表内はO(d)。表外は積の総項数sとしてO(d+s)回の係数演算と高々d回の逆元計算。

### BinomialMod::H

```text
value_type H(index_type n, index_type k);
```

重複組合せnHkを返します。

**制約:** 負のnまたはkでは0。0H0=1、0Hk=0（k>0）。通常のCOMへ渡すn+k-1はint64_t内。法1なら0。

**計算量:** N=n+k-1としたCOM(N,k)と同じ。素数法でN<法かつ準備済みならO(1)、表外と自動拡張は対応する費用。


次の例では利用側で`<atcoder/modint>`を読み込みます。ライブラリ本体にMint実装は含みません。

```cpp
using Mint = atcoder::modint998244353;
sirkit::ModIntBinomial<Mint> comb(100);
const auto ways = comb.COM(10, 3); // ways.val() == 120。
const auto multi = comb.H(3, 4);  // multi.val() == 15。
sirkit::BinomialMod integer(72, 100);
const auto residue = integer.COM(10, 5); // 36。
```

動的modintの法は構築前に設定し、法を変えたら計算器も作り直します。`COM`は法に応じて方式を選び、`n<0`・`k<0`・`k>n`や法1では0です。`Lucas`は素数法専用です。素数専用の`PrimeBinomial`はこのファイルには含みません。

法をm、異なる素因子数をr、全表の総要素数をBとすると、構築はO(sqrt(m)+B+r log m)時間、`prepare`はO(B+r log m)時間、保持領域はO(B+r)です。準備済み素数法の`n<m`はO(1)。Lucasの表内はd桁としてO(d)、表外は積の総項数sとしてO(d+s)回のMint演算と高々d回の逆元計算です。通常のmodintの逆元計算はO(log m)です。合成数法では同梱の整数版を用います。

前計算の既定値は500万です。`BinomialOptions`で表と積の上限を指定でき、表外照会では表の自動拡張費用が加わる場合があります。表を持たない場合は`max_n=0`と`table_limit=0`を指定します。

[完全な使用例](../../Examples/combination_modint_example.cpp)・[方式・表外の計算量・上限の詳細](../binomial_coefficient.md)
