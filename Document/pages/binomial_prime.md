# 二項係数（素数法・Mint）

`PrimeBinomial<Mint>`は、素数の法に限定した二項係数計算器です。利用側が用意するAtCoder modint互換型の値で返します。合成数法用の計算器や`BinomialOptions`は含みません。

計算量のmは法、rは異なる素因子数、Bは全表の総要素数、dはLucasの桁数、sは表外の積の総項数です。通常のMintの逆元計算はO(log m)です。

## API

### PrimeBinomial::PrimeBinomial

```text
explicit PrimeBinomial(std::int64_t max_n = 5'000'000, std::uint64_t max_steps = 1'000'000);
```

前計算範囲と表外の積の回数上限を指定して構築。

**制約:** max_n>=0。Mintの法は素数。動的Mintの法は構築前に設定し、変更後は再構築。

**計算量:** 前計算B要素にO(B)回のMint演算と高々1回の逆元計算、O(B)領域。

### PrimeBinomial::prepare

```text
void prepare(std::int64_t max_n = 5'000'000);
```

指定した範囲の計算用表を準備します。

**制約:** max_n>=0。準備範囲はmin(max_n,mod-1)と内部容量の範囲。

**計算量:** 前計算B要素にO(B)回のMint演算と高々1回の逆元計算、O(B)領域。

### PrimeBinomial::COM

```text
Mint COM(std::int64_t n, std::int64_t k) const;
```

二項係数nCkを法の剰余で返します。

**制約:** n,kはint64_t。n<0・k<0・k>nは0。動的Mintの法は途中で変更しない。

**計算量:** 素数法でn<法かつ準備済みならO(1)。Lucasはd桁と表外の積s項にO(d+s)回のMint演算と高々d回の逆元計算。任意法版は整数版に従う。

### PrimeBinomial::Lucas

```text
Mint Lucas(std::int64_t n, std::int64_t k) const;
```

素数法でLucasの定理によるnCkを返します。

**制約:** PrimeBinomialの法は素数。合成数法には任意法クラスを使う。

**計算量:** d桁の表内はO(d)。表外は積の総項数sとしてO(d+s)回の係数演算と高々d回の逆元計算。

### PrimeBinomial::H

```text
Mint H(std::int64_t n, std::int64_t k) const;
```

重複組合せnHkを返します。

**制約:** 負のnまたはkでは0。0H0=1、0Hk=0（k>0）。通常のCOMへ渡すn+k-1はint64_t内。法1なら0。

**計算量:** N=n+k-1としたCOM(N,k)と同じ。素数法でN<法かつ準備済みならO(1)、表外と自動拡張は対応する費用。


次の例では利用側で`<atcoder/modint>`を読み込みます。

```cpp
using Mint = atcoder::modint998244353;
sirkit::PrimeBinomial<Mint> comb(100);
const auto ways = comb.COM(10, 3); // ways.val() == 120。
const auto multi = comb.H(3, 4);  // multi.val() == 15。
comb.prepare(200);               // 表を広げる場合は明示する。
```

`COM`は`n<0`・`k<0`・`k>n`で0です。`n>=p`はLucasの定理で計算し、`Lucas`を明示的に呼んでも同じ結果です。`H`は重複組合せで、`0H0=1`、`0Hk=0`（k>0）です。動的modintの法を変えたら計算器も作り直します。

pを法、Bを準備後の表の長さとすると、構築・`prepare`はO(B)回のMint演算と高々1回の逆元計算、O(B)領域です。準備済みの`n<p`はO(1)。Lucasはd=O(1+log_p(n+1))桁として、表内はO(d)、表外は積の総項数sに対してO(d+s)回のMint演算と高々d回の逆元計算です。通常のmodintの逆元計算はO(log p)です。

前計算の既定値は500万で、実際には`min(max_n,p-1)`までを準備します。表外照会は積で求め、表を自動では広げません。構築時の`max_steps`は1照会の積の回数上限で、超過すると`length_error`です。コピー・ムーブ構築に対応しますが代入には対応しません。

[完全な使用例](../../Examples/combination_prime_example.cpp)・[共通の方式と所有・失敗時の前提](../binomial_coefficient.md)
