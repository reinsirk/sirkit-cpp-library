# ゼータ／メビウス変換と整数篩

集合・約数・倍数の総和変換、AND／OR・GCD／LCM畳み込み、整数篩をまとめた全機能版です。必要な機能だけ使う場合は[用途別版](../Math/Zeta/README.md)を選べます。

Mは篩の上限、pi(M)はM以下の素数の個数、Dは約数の個数です。

変換・畳み込みのnは配列長です。疎変換のmは要素数、rは各keyの素因数を重複込みで数えた総数です。

## API

### superset_zeta_transform

```text
template <class T> void superset_zeta_transform(std::vector<T>& f);
```

各集合の値を上位集合の総和へ更新します。

**制約:** 空または2冪長。値型は可換な加算に対応。

**計算量:** O(1+n log(n+1))時間・O(1)追加領域。

### superset_mobius_transform

```text
template <class T> void superset_mobius_transform(std::vector<T>& f);
```

上位集合ゼータ変換を元へ戻します。

**制約:** 空または2冪長。値型は加法逆元を持つ。

**計算量:** O(1+n log(n+1))時間・O(1)追加領域。

### subset_zeta_transform

```text
template <class T> void subset_zeta_transform(std::vector<T>& f);
```

各集合の値を部分集合の総和へ更新します。

**制約:** 空または2冪長。値型は可換な加算に対応。

**計算量:** O(1+n log(n+1))時間・O(1)追加領域。

### subset_mobius_transform

```text
template <class T> void subset_mobius_transform(std::vector<T>& f);
```

部分集合ゼータ変換を元へ戻します。

**制約:** 空または2冪長。値型は加法逆元を持つ。

**計算量:** O(1+n log(n+1))時間・O(1)追加領域。

### bitwise_and_convolution

```text
template <class T> std::vector<T> bitwise_and_convolution(std::vector<T> f, std::vector<T> g);
```

添字のANDがkとなる積の総和を返します。

**制約:** 空または2冪長。同長入力（自己畳み込みは1本）。係数は環型。

**計算量:** O(1+n log(n+1))時間・O(n+1)領域。

### bitwise_or_convolution

```text
template <class T> std::vector<T> bitwise_or_convolution(std::vector<T> f, std::vector<T> g);
```

添字のORがkとなる積の総和を返します。

**制約:** 空または2冪長。同長入力（自己畳み込みは1本）。係数は環型。

**計算量:** O(1+n log(n+1))時間・O(n+1)領域。

### bitwise_and_square

```text
template <class T> std::vector<T> bitwise_and_square(std::vector<T> f);
```

同じ配列を2入力とするAND畳み込みを返します。

**制約:** 空または2冪長。同長入力（自己畳み込みは1本）。係数は環型。

**計算量:** O(1+n log(n+1))時間・O(n+1)領域。

### bitwise_or_square

```text
template <class T> std::vector<T> bitwise_or_square(std::vector<T> f);
```

同じ配列を2入力とするOR畳み込みを返します。

**制約:** 空または2冪長。同長入力（自己畳み込みは1本）。係数は環型。

**計算量:** O(1+n log(n+1))時間・O(n+1)領域。

### IntegerSieve::IntegerSieve

```text
explicit IntegerSieve(int maxn);
IntegerSieve(const IntegerSieve&);
IntegerSieve(IntegerSieve&& other) noexcept;
```

最小素因数表と昇順の素数列を作ります。コピーは元の表から独立し、移動元は空になります。

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

### sparse_multiple_zeta_transform

```text
template <std::integral Int, class Val> void sparse_multiple_zeta_transform(std::vector<std::pair<Int, Val>>& f, const IntegerSieve& sieve);
```

疎なkey集合の値を倍数和へ更新し、key昇順に並べます。

**制約:** keyは正・一意・全約数を含み、篩の上限以下。

**計算量:** 要素数m・素因数重複総数rにO((m+r)log(m+1))時間・O(m+r)領域。

### sparse_multiple_mobius_transform

```text
template <std::integral Int, class Val> void sparse_multiple_mobius_transform(std::vector<std::pair<Int, Val>>& f, const IntegerSieve& sieve);
```

疎な倍数ゼータ変換を元へ戻し、key昇順に並べます。

**制約:** keyは正・一意・全約数を含み、篩の上限以下。

**計算量:** 要素数m・素因数重複総数rにO((m+r)log(m+1))時間・O(m+r)領域。


## 集合の変換・AND／OR畳み込み

入力は空配列またはn=2^k要素です。畳み込みの2入力は同じ長さにします。


各O(1+n log(n+1))時間です。4変換は入力を直接更新しO(1)追加領域、畳み込みはO(n+1)領域です。

## IntegerSieve

`IntegerSieve(M)`はM>=0の最小素因数表と昇順素数列をO(M+1)時間・領域で作ります。公開表`least_prime_factor`・`prime_list`は変更せず使います。0・1の最小素因数は0です。


Dは約数の個数、pi(M)はM以下の素数の個数です。Mintは利用側で用意します。計算量では乗算をO(1)、powをO(log(k+1))と数えます。

## 約数・倍数の変換

`multiple_zeta_transform(f)`は各dの倍数の総和、`divisor_zeta_transform(f)`は各kの約数の総和へ直接更新します。`multiple_mobius_transform(f)`・`divisor_mobius_transform(f)`は各逆変換です。空配列も扱い、f[0]は不変です。

各`(f,sieve)`版は`sieve.max_value()>=f.size()-1`の表を再利用し、n=f.size()としてO(1+n log log(n+2))時間・O(1)追加領域です。`(f)`版はn-1<=INT_MAXを前提に表も作り、O(n+1)追加領域です。

## GCD／LCM畳み込み

`gcd_convolution(f,g)`・`lcm_convolution(f,g)`は、gcd(i,j)／lcm(i,j)ごとに積の総和を並べたvectorを返します。`gcd_square(f)`・`lcm_square(f)`はg=fの場合です。両方の入力は同じ長さとし、空配列も扱えます。

`gcd(0,k)=k`、`gcd(0,0)=0`、`lcm(0,k)=0`とし、LCMが結果長n以上の組は切り捨てます。各`(f,g,sieve)`・`(f,sieve)`版もあり、篩の前提は約数・倍数変換と同じです。O(1+n log log(n+2))時間・O(n+1)追加領域です。

## 疎な倍数変換

`sparse_multiple_zeta_transform(f,sieve)`・`sparse_multiple_mobius_transform(f,sieve)`は`vector<pair<Int,Val>>`の値を倍数和／その逆へ更新し、key昇順に並べます。keyは正で一意、全約数も含み、key<=sieve.max_value()を満たします。

要素数m、各keyの素因数を重複込みで数えた総数rにO((m+r)log(m+1))時間・O(m+r)追加領域です。初めからkeyが1..m順ならO(m log log(m+2))時間・O(1)追加領域です。

## 値型・更新と検査

値型はbool以外で、使う加減乗算と零の構築に対応します。ゼータ変換は可換な加算、逆変換は加法逆元、畳み込みは分配法則が前提です。符号付き整数は途中結果も型の範囲内、符号なし整数は型の幅の剰余です。浮動小数点には丸め誤差があります。

変換は入力を直接更新し、値演算・並べ替えの例外では部分更新が残ることがあります。畳み込みは値渡しで、lvalue入力を保持しrvalueの領域を移譲できます。同じ配列を`(std::move(f),f)`で渡さず自己畳み込み版を使ってください。

`SKIP_RUNTIME_CHECKS`でも配列長・篩上限・key集合の前提は同じです。未参照部分までの全入力検査はしません。

```cpp
std::vector<std::int64_t> a{1,2}, b{3,4};
auto h = bitwise_and_convolution(a,b); // {13,8}。
subset_zeta_transform(a);             // {1,3}。
subset_mobius_transform(a);           // {1,2}。
```

[完全な使用例](../Examples/zeta_mobius_example.cpp)・[内部処理と検証の記録](../Audit/Changes/library-docs-concise-math-geometry-2026-11-15.json)
