# AND／OR畳み込み

`bitwise_and_convolution(f,g)`は`h[k]=sum_{i&j=k}f[i]g[j]`、`bitwise_or_convolution(f,g)`は`h[k]=sum_{i|j=k}f[i]g[j]`を返します。1本の配列を2入力に使う場合は`bitwise_and_square`・`bitwise_or_square`も使えます。4つの集合ゼータ／メビウス変換も同梱します。

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


次の例では利用側で`<cstdint>`も読み込みます。

```cpp
const std::vector<std::int64_t> a{1,2}, b{3,4};
const auto h = sirkit::bitwise_and_convolution(a,b); // {13,8}。
const auto s = sirkit::bitwise_or_square(a);         // {1,8}。
```

2入力は同じ長さで、空配列またはn=2^k要素です。空なら空配列を返します。係数は`bool`以外の環型で、加減算・乗算を提供します。符号付き整数の途中の和・差・積も型の範囲内とし、符号なし整数は型の幅の剰余として計算します。

各畳み込みはO(1+n log(n+1))時間・O(n+1)領域です。通常の2入力版はゼータ変換2回と逆変換1回、自己畳み込みは各1回です。値渡しのためlvalue入力はコピーして保持し、不要な入力は`std::move`で領域を譲渡できます。同じ配列を`(std::move(f),f)`として渡すことは避け、自己畳み込み版を使います。

同梱の4変換は入力を直接更新し、O(1+n log(n+1))時間・O(1)追加領域です。このファイルに整数篩やGCD／LCM畳み込みは含みません。

[完全な使用例](../../Examples/zeta_bitwise_example.cpp)・[集合変換と畳み込みの共通説明](../zeta_mobius.md)
