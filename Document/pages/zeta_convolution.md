# GCD／LCM畳み込み

`gcd_convolution(f,g)`は`h[k]=sum_{gcd(i,j)=k}f[i]g[j]`、`lcm_convolution(f,g)`は`h[k]=sum_{lcm(i,j)=k}f[i]g[j]`を返します。自己畳み込みの`gcd_square`・`lcm_square`、必要な整数篩と約数・倍数変換も同梱しています。

Mは篩の上限、pi(M)はM以下の素数数、Dは約数の個数です。

変換・畳み込みのnは配列長です。疎変換のmは要素数、rは各keyの素因数を重複込みで数えた総数です。

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

### gcd_convolution

```text
template <class T> std::vector<T> gcd_convolution(std::vector<T> f, std::vector<T> g, const IntegerSieve& sieve);
template <class T> std::vector<T> gcd_convolution(std::vector<T> f, std::vector<T> g);
```

添字のGCDがkとなる積の総和を返します。

**制約:** 同長入力（自己畳み込みは1本）で環型。表ありはmax_value()>=n-1、表なしはn-1<=INT_MAX。

**計算量:** O(1+n log log(n+2))時間・O(n+1)領域。

### lcm_convolution

```text
template <class T> std::vector<T> lcm_convolution(std::vector<T> f, std::vector<T> g, const IntegerSieve& sieve);
template <class T> std::vector<T> lcm_convolution(std::vector<T> f, std::vector<T> g);
```

添字のLCMがkとなる積の総和を返します。

**制約:** 同長入力（自己畳み込みは1本）で環型。表ありはmax_value()>=n-1、表なしはn-1<=INT_MAX。

**計算量:** O(1+n log log(n+2))時間・O(n+1)領域。

### gcd_square

```text
template <class T> std::vector<T> gcd_square(std::vector<T> f, const IntegerSieve& sieve);
template <class T> std::vector<T> gcd_square(std::vector<T> f);
```

同じ配列を2入力とするGCD畳み込みを返します。

**制約:** 同長入力（自己畳み込みは1本）で環型。表ありはmax_value()>=n-1、表なしはn-1<=INT_MAX。

**計算量:** O(1+n log log(n+2))時間・O(n+1)領域。

### lcm_square

```text
template <class T> std::vector<T> lcm_square(std::vector<T> f, const IntegerSieve& sieve);
template <class T> std::vector<T> lcm_square(std::vector<T> f);
```

同じ配列を2入力とするLCM畳み込みを返します。

**制約:** 同長入力（自己畳み込みは1本）で環型。表ありはmax_value()>=n-1、表なしはn-1<=INT_MAX。

**計算量:** O(1+n log log(n+2))時間・O(n+1)領域。


```cpp
const std::vector<std::int64_t> a{0,1,2,3}, b{0,4,5,6};
const auto g = sirkit::gcd_convolution(a,b); // {0,62,10,18}。
const auto l = sirkit::lcm_convolution(a,b); // {0,4,23,36}。
```

入力する配列はどちらも長さnです。入力が空なら空配列を返します。`gcd(0,k)=k`・`gcd(0,0)=0`・`lcm(0,k)=0`として0の寄与も含めます。LCMがn以上になる組は結果に含めません。

各畳み込み・自己畳み込みはO(1+n log log(n+2))時間・O(n+1)領域です。`(f,g,sieve)`版と`(f,sieve)`版は`sieve.max_value()>=n-1`の表を再利用し、表なし版は`n-1<=INT_MAX`が前提です。自己畳み込みは値渡しの入力を1本にし、ゼータ変換と逆変換を各1回にします。

値型Tは`bool`以外で、`T{}`を零として加減算・乗算ができる環型です。符号付き整数の途中の値も型の範囲内とし、符号なし整数は型の幅の剰余として扱います。lvalue入力はコピーして保持し、不要な入力は`std::move`で渡せます。

同梱の[整数篩](zeta_sieve.md)・[約数・倍数変換](zeta_integer.md)も参照してください。

[完全な使用例](../../Examples/zeta_convolution_example.cpp)・[畳み込みの共通説明](../zeta_mobius.md)
