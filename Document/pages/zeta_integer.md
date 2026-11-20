# 約数・倍数のゼータ／メビウス変換

`divisor_zeta_transform`は`f[k]=sum_{d|k}f[d]`、`multiple_zeta_transform`は`f[d]=sum_{d|k}f[k]`へ入力配列を直接更新します。対応する`mobius`操作はそれぞれの逆変換です。`IntegerSieve`とその数論関数も同梱しています。

Mは篩の上限、pi(M)はM以下の素数の個数、Dは約数の個数です。

変換・畳み込みのnは配列長です。疎変換のmは要素数、rは各keyの素因数を重複込みで数えた総数です。

## API

### multiple_zeta_transform

```text
template <class T> void multiple_zeta_transform(std::vector<T>& f, const IntegerSieve& sieve);
template <class T> void multiple_zeta_transform(std::vector<T>& f);
```

各添字の値を倍数の総和へ更新します。

**制約:** 素数表付きはsieve.max_value()>=f.size()-1。表なしはf.size()-1<=INT_MAX。

**計算量:** O(1+n log log(n+2))時間。表なし版はO(n+1)、表再利用の変換はO(1)追加領域。

### multiple_mobius_transform

```text
template <class T> void multiple_mobius_transform(std::vector<T>& f, const IntegerSieve& sieve);
template <class T> void multiple_mobius_transform(std::vector<T>& f);
```

倍数ゼータ変換を元へ戻します。

**制約:** 素数表付きはsieve.max_value()>=f.size()-1。表なしはf.size()-1<=INT_MAX。

**計算量:** O(1+n log log(n+2))時間。表なし版はO(n+1)、表再利用の変換はO(1)追加領域。

### divisor_zeta_transform

```text
template <class T> void divisor_zeta_transform(std::vector<T>& f, const IntegerSieve& sieve);
template <class T> void divisor_zeta_transform(std::vector<T>& f);
```

各添字の値を約数の総和へ更新します。

**制約:** 素数表付きはsieve.max_value()>=f.size()-1。表なしはf.size()-1<=INT_MAX。

**計算量:** O(1+n log log(n+2))時間。表なし版はO(n+1)、表再利用の変換はO(1)追加領域。

### divisor_mobius_transform

```text
template <class T> void divisor_mobius_transform(std::vector<T>& f, const IntegerSieve& sieve);
template <class T> void divisor_mobius_transform(std::vector<T>& f);
```

約数ゼータ変換を元へ戻します。

**制約:** 素数表付きはsieve.max_value()>=f.size()-1。表なしはf.size()-1<=INT_MAX。

**計算量:** O(1+n log log(n+2))時間。表なし版はO(n+1)、表再利用の変換はO(1)追加領域。

### IntegerSieve::IntegerSieve

```text
explicit IntegerSieve(int maxn);
IntegerSieve(const IntegerSieve&);
IntegerSieve(IntegerSieve&& other) noexcept;
```

最小素因数の表と、素数を昇順に並べた列を構築します。コピーは独立し、移動元は空になります。

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
std::vector<std::int64_t> a{0,1,2,3,4};
sirkit::IntegerSieve sieve(4);
sirkit::divisor_zeta_transform(a, sieve);   // {0,1,3,4,7}。
sirkit::divisor_mobius_transform(a, sieve); // {0,1,2,3,4}。
```

正の添字を変換し、`f[0]`は不変です。空配列も扱います。nを配列長とすると、各変換はO(1+n log log(n+2))時間です。`(f,sieve)`版は`sieve.max_value()>=n-1`の表を再利用してO(1)追加領域、`(f)`版は篩も構築してO(n)追加領域を使い、`n-1<=INT_MAX`が前提です。

ゼータ変換には加算、逆変換には減算が必要です。符号付き整数の途中の値は型の範囲内とし、符号なし整数は型の幅の剰余として扱います。入力は直接変更するため、値演算が例外を送出すると部分更新が残ることがあります。

同梱の整数篩は[IntegerSieveの説明](zeta_sieve.md)を参照してください。このファイルにGCD／LCM畳み込みや疎なkey集合の変換は含みません。

[完全な使用例](../../Examples/zeta_integer_example.cpp)・[整数変換の共通説明](../zeta_mobius.md)
