# 分割数・ベル数・スターリング数・撹乱順列

整数や集合を分割する方法の数、固定点を持たない順列の数、ベルヌーイ数やべき乗和を計算します。

```cpp
auto partitions = sirkit::partition_numbers(5);  // {1, 1, 2, 3, 5, 7}
auto bell = sirkit::bell_numbers(4);              // {1, 1, 2, 5, 15}
```

## `partition_numbers`

```text
std::vector<std::uint32_t> partition_numbers(std::size_t n);
```

各整数 $0,1,\ldots,n$ を正整数の和で表す方法の数を返します。足す順序は無視します。$O((n+1)\log(n+2))$ 時間です。 法998244353、$0\le n<2^{22}$、領域 $O(n+1)$ です。

## `bell_numbers`

```text
std::vector<std::uint32_t> bell_numbers(std::size_t n);
```

要素が $0,1,\ldots,n$ 個の集合を、空でないブロックに分ける方法の数を返します。$O((n+1)\log(n+2))$ 時間です。 法998244353、$0\le n<2^{22}$、領域 $O(n+1)$ です。

## `stirling_first_row`

```text
std::vector<std::uint32_t> stirling_first_row(std::size_t n);
```

下降階乗 $x(x-1)\cdots(x-n+1)$ の係数を定数項から返します。符号付き第1種スターリング数にあたります。$O((n+1)\log^2(n+2))$ 時間です。 法998244353、$0\le n<2^{22}$、領域 $O(n+1)$ です。

## `stirling_second_row`

```text
std::vector<std::uint32_t> stirling_second_row(std::size_t n);
```

$n$ 個の要素を $k$ 個の空でないブロックに分ける方法の数を、$k=0$ から順に返します。$O((n+1)\log(n+2))$ 時間です。 法998244353、$0\le n<2^{22}$、領域 $O(n+1)$ です。

## `derangement_numbers`

```text
std::vector<std::uint32_t> derangement_numbers(std::size_t n, std::uint32_t mod = 998244353);
```

元の位置に残る要素がない順列の数を、長さ0から `n` まで返します。法は正の32ビット符号なし整数です。$O(n+1)$ 時間・領域です。

## `bernoulli_numbers`

```text
std::vector<std::uint32_t> bernoulli_numbers(std::size_t n);
```

$B_0$ から $B_n$ を返します。指数型母関数 $x/(\exp(x)-1)$ に従い、$B_1=-1/2$ です。法は998244353、$0\le n<2^{22}$、$O((n+1)\log(n+2))$ 時間・$O(n+1)$ 領域です。

## `sum_of_powers`

```text
std::uint32_t sum_of_powers(std::uint64_t n, std::size_t exponent);
```

$1^k+\cdots+n^k$ の法998244353での値です。指数 `exponent` は $0\le k<2^{22}$。$k=0$ では $n$ の余り、$n=0$ では空和の $0$ を返します。計算量は $O((k+1)\log(k+2))$ 時間・$O(k+1)$ 領域。

## `subset_sum_counts`

```text
std::vector<std::uint32_t> subset_sum_counts(std::span<const std::size_t> values,
                                             std::size_t max_sum);
```

合計 $0$ から `max_sum` までについて、その和を作る添字集合の個数を返します。同じ値も添字ごとに区別し、空集合も数えます。法は998244353、$m=\texttt{max_sum}<2^{22}$ です。入力 $n$ 要素に対して $O(n+(m+1)\log(m+2))$ 時間・$O(m+1)$ 領域です。

## 同梱の形式的冪級数・NTT

`FormalPowerSeries` の四則演算・微積分・精度を指定する演算と、`NTT<Mod, Root>`、`ConvolutionMod` も含みます。[FormalPowerSeriesのAPI](formal_power_series.md#api) ／ [NTTとConvolutionModのAPI](ntt.md#api)

[使用例](../Examples/combinatorial_sequences_example.cpp) ／ [分割数のVerify](../Verify/current/submissions/partition_function.cpp) ／ [ベルヌーイ数のVerify](../Verify/current/submissions/bernoulli_number.cpp)
