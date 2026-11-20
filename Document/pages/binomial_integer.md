# 二項係数（整数・任意法）

`BinomialMod`は、法`1..UINT32_MAX`の二項係数を整数で返します。合成数の法にも対応し、Mint実装を用意する必要はありません。`n,k`は`std::int64_t`です。

BinomialModの`index_type`は`std::int64_t`、`value_type`は`std::uint32_t`です。

計算量のmは法、rは異なる素因子数、Bは全表の総要素数、dはLucasの桁数、sは表外の積の総項数です。通常のMintの逆元計算はO(log m)です。

## API

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


```cpp
sirkit::BinomialMod comb(72, 100);
const auto ways = comb.COM(10, 5); // 252 mod 72 = 36。
const auto multi = comb.H(3, 2);   // 6。
const auto modulus = comb.mod();  // 72。
```

`COM(n,k)`は`n<0`・`k<0`・`k>n`で0、法1でも常に0です。`H(n,k)`は重複組合せで、`0H0=1`（法1では0）、`0Hk=0`（k>0）です。`Lucas`は素数法専用で、合成数や法1では`domain_error`です。通常の照会には法に応じて方式を選ぶ`COM`を使えます。

法をm、異なる素因子数をr、全表の総要素数をBとすると、構築はO(sqrt(m)+B+r log m)時間、`prepare`はO(B+r log m)時間、保持領域はO(B+r)です。準備済みの素数法で`n<m`なら照会はO(1)、準備済みの合成数法はO(Σ_p(1+log_p(n+1)))です。表外は積による計算を使い、自動拡張の費用も加わります。

`BinomialOptions.table_limit`は各表の最大添字、`max_multiplicative_steps`は1照会の積の回数上限です。表を持たない場合は構築時の`max_n=0`に加えて`table_limit=0`も指定します。計算予算・容量保護・数学的な0判定は`SKIP_RUNTIME_CHECKS`でも維持します。

[完全な使用例](../../Examples/combination_integer_example.cpp)・[方式・表外の計算量・上限の詳細](../binomial_coefficient.md)
