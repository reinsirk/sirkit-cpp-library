# 整数篩・素因数分解・数論関数表

`IntegerSieve(M)`は、0からMまでの最小素因数表と昇順の素数列を作ります。素数判定・素因数分解・約数列挙・メビウス関数表・Eulerのφ関数表などを扱います。

Mは篩の上限、pi(M)はM以下の素数数、Dは約数の個数です。

## API

### IntegerSieve::IntegerSieve

```text
explicit IntegerSieve(int maxn);
IntegerSieve(const IntegerSieve&);
IntegerSieve(IntegerSieve&& other) noexcept;
```

最小素因数表と昇順素数列を構築します。コピーは独立、移動元は空です。

**制約:** 0<=maxnで内部容量の上限内。

**計算量:** O(M+1)時間・領域。移動構築はO(1)。

### IntegerSieve::operator=

```text
IntegerSieve& operator=(const IntegerSieve& other);
IntegerSieve& operator=(IntegerSieve&& other) noexcept;
```

コピーまたは移動で内容を置き換え、自身への参照を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** コピーO(M+1)、移動は旧領域の解放を含む。

### IntegerSieve::swap

```text
void swap(IntegerSieve& other) noexcept;
```

内容と所有する領域を交換します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### IntegerSieve::max_value

```text
int max_value() const noexcept;
```

篩の上限を返します。空の表では-1です。

**制約:** なし。

**計算量:** O(1)。

### IntegerSieve::is_prime

```text
template <std::integral Int> bool is_prime(Int x) const;
```

素数ならtrueを返します。

**制約:** 0<=x<=max_value()。

**計算量:** O(1)。

### IntegerSieve::prime_factorization

```text
template <std::integral Int> std::map<Int, int> prime_factorization(Int x) const;
```

素因数と指数のmapを返します。

**制約:** 1<=x<=M²。x=1は空の表でも可。

**計算量:** 表内O(log x)、表外は最悪O(pi(M)+log x)。

### IntegerSieve::enumerate_divisors

```text
template <std::integral Int> std::vector<Int> enumerate_divisors(Int x) const;
```

全ての正の約数をvectorで返します。

**制約:** 1<=x<=M²。x=1は空の表でも可。

**計算量:** 素因数分解に加えO(D)。

### IntegerSieve::sorted_divisors

```text
template <std::integral Int> std::vector<Int> sorted_divisors(Int x) const;
```

全ての正の約数を昇順のvectorで返します。

**制約:** 1<=x<=M²。x=1は空の表でも可。

**計算量:** 素因数分解に加えO(D log(D+1))。

### IntegerSieve::divisor_totients

```text
template <std::integral Int> std::map<Int, Int> divisor_totients(Int x) const;
```

各約数dとφ(d)のmapを返します。

**制約:** 1<=x<=M²。x=1は空の表でも可。

**計算量:** 素因数分解に加えO(D log(D+1))。

### IntegerSieve::mobius_table

```text
std::vector<int> mobius_table() const;
```

メビウス関数の表を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(M+1)時間・領域。

### IntegerSieve::totient_table

```text
std::vector<int> totient_table() const;
```

Eulerのφ関数の表を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(M+1)時間・領域。

### IntegerSieve::power_table

```text
template <class MODINT> std::vector<MODINT> power_table(std::int64_t k, int nmax) const;
```

0から指定上限までの整数のk乗をMintで返します。

**制約:** k>=0、0<=nmax<=M。利用側でMintを用意する。

**計算量:** O(nmax+pi(nmax)log(k+1))時間・O(nmax+1)領域。


```cpp
sirkit::IntegerSieve sieve(100);
const auto prime = sieve.is_prime(97);            // true。
const auto factors = sieve.prime_factorization(84); // {{2,2},{3,1},{7,1}}。
const auto divisors = sieve.sorted_divisors(12);  // {1,2,3,4,6,12}。
const auto phi = sieve.totient_table();           // phi[12] == 4。
```

Mは非負で、構築はO(M+1)時間・領域です。公開配列`least_prime_factor`と`prime_list`は変更せず利用します。`is_prime(x)`は`0<=x<=M`をO(1)で判定します。0と1の最小素因数は0です。

素因数分解と約数関連のAPIは`1<=x<=M²`が対象で、x=1だけは空の表でも扱えます。表内の素因数分解はO(log x)、表外は最悪O(pi(M)+log x)です。pi(M)はM以下の素数の個数です。約数をd個とすると、列挙は分解に加えてO(d)、昇順列挙・各約数のφ値のmap構築はO(d log(d+1))時間を使います。

`mobius_table`・`totient_table`はO(M)時間・領域です。`power_table<Mint>(k,nmax)`には利用側でMintを用意し、O(nmax+pi(nmax)log(k+1))時間・O(nmax)領域でべき乗表を作ります。`0^0=1`です。コピーは独立し、ムーブ元は空になります。

[完全な使用例](../../Examples/zeta_sieve_example.cpp)・[整数篩の共通説明](../zeta_mobius.md)
