# 疎な倍数ゼータ／メビウス変換

`vector<pair<Int,Val>>`のkeyが倍数関係にある要素同士で値をまとめます。`sparse_multiple_zeta_transform(f,sieve)`は各keyの値を倍数和へ置換し、`sparse_multiple_mobius_transform`で元へ戻します。両操作ともkeyの昇順に並べ替えます。整数篩も同梱しています。

Mは篩の上限、pi(M)はM以下の素数の個数、Dは約数の個数です。

変換・畳み込みのnは配列長です。疎変換のmは要素数、rは各keyの素因数を重複込みで数えた総数です。

## API

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
sirkit::IntegerSieve sieve(6);
std::vector<std::pair<int,std::int64_t>> a{{1,1},{2,2},{3,3},{6,6}};
sirkit::sparse_multiple_zeta_transform(a,sieve);   // {{1,12},{2,8},{3,9},{6,6}}。
sirkit::sparse_multiple_mobius_transform(a,sieve); // 元の値へ戻す。
```

keyはそれぞれ異なる正の値とし、各keyのすべての約数もkeyの集合に含めます。各keyは`key<=sieve.max_value()`を満たす必要があります。空配列も扱います。値型はゼータ変換で加算、逆変換で減算に対応する必要があります。

要素数をm、各keyの素因数を重複込みで数えた総数をrとすると、O((m+r)log(m+1))時間・O(m+r)追加領域です。keyが初めから`1,2,...,m`の順ならO(m log log(m+2))時間・O(1)追加領域です。篩の構築費用は別で、`IntegerSieve(M)`はO(M+1)時間・領域を使います。

入力を直接変更するため、並べ替えや値演算の例外では途中の状態が残ることがあります。符号付き整数の途中の値は型の範囲内とし、符号なし整数は型の幅の剰余として扱います。同梱する[整数篩の操作](zeta_sieve.md)は公開表を変更せず利用してください。

[完全な使用例](../../Examples/zeta_sparse_example.cpp)・[疎変換の共通説明](../zeta_mobius.md)
