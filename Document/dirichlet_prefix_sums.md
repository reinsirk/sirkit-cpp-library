# Dirichlet積と累積和

数列の累積和が、$N$ を正整数で割った商の位置だけで分かっているときに使います。その座標を変えずに Dirichlet 積、商、逆元の累積和を求めます。例えば、すべての項が 1 の列の逆元はメビウス関数です。

## 構築

```text
template <class T>
explicit DirichletPrefixSums(std::uint64_t n)
```

正整数 $N$ を渡します。$T$ は可換環の値型です。既定値が 0 で、`T{1}` が 1 となり、`+`、`-`、`*` と複合代入 `+=`、`-=` を使える必要があります。商・逆元では等値比較も使います。整数型なら計算途中も型の範囲内に収まるようにします。構築は $O(\log\log(N+2))$ 時間・$O(1)$ 領域です。

## `size`

```text
std::uint64_t size() const noexcept
```

異なる $\lfloor N/i\rfloor$ の個数を返します。$O(1)$ 時間です。

## `quotient`

```text
std::uint64_t quotient(std::uint64_t index) const
```

商を昇順に並べたときの位置 `index` の値を返します。添字は 0 始まりで、`size()` 未満です。$O(1)$ 時間です。

## `convolution`

```text
std::vector<T> convolution(std::span<const T> left, std::span<const T> right) const
```

2 列の累積和を受け取り、Dirichlet 積 $(f*g)(k)=\sum_{ij=k}f(i)g(j)$ の累積和を返します。入出力は `quotient` の順に `size()` 要素です。$O(N^{2/3})$ 回の $T$ 演算と $O(\sqrt N)$ の追加領域を使います。

## `divide`

```text
std::vector<T> divide(std::span<const T> product, std::span<const T> divisor, T inverse_at_one) const
```

`divisor` との Dirichlet 積が `product` になる列の累積和を返します。入出力の座標と計算量は `convolution` と同じです。`inverse_at_one` に `divisor[0]` の乗法逆元を渡します。

## `inverse`

```text
std::vector<T> inverse(std::span<const T> divisor, T inverse_at_one) const
```

Dirichlet 積の逆元の累積和を返します。どの座標でも積の累積和が 1 になる `divide` です。座標、`inverse_at_one` の条件、計算量は `divide` と同じです。

```cpp
const sirkit::DirichletPrefixSums<std::int64_t> layout(10);
const std::vector<std::int64_t> ones{1, 2, 3, 5, 10};
const auto divisors = layout.convolution(ones, ones); // 最後の値が 27
const auto mobius = layout.inverse(ones, 1);         // 最後の値が -1
```

[使用例](../Examples/dirichlet_prefix_sums_example.cpp) ／ [積のVerify](../Verify/current/submissions/dirichlet_convolution_and_prefix_sums.cpp) ／ [逆元のVerify](../Verify/current/submissions/dirichlet_inverse_and_prefix_sums.cpp)
