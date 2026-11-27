# 集合冪級数と部分集合畳み込み

部分集合ごとに持つ値を、要素が重複しない分け方で合成します。集合の分割を数えるときは指数化を使えます。

```cpp
const std::vector<std::uint32_t> a{0, 2, 3, 4};
auto f = sirkit::set_series_exp(a);  // {1, 2, 3, 10}
```

## `subset_convolution`

```text
std::vector<std::uint32_t> subset_convolution(
    std::span<const std::uint32_t> a, std::span<const std::uint32_t> b);
```

集合 `s` の答えは、`a[t] * b[s ^ t]` を `s` のすべての部分集合 `t` について足した値です。`a` と `b` は同じ長さを渡します。 係数列はビットマスク順で、長さは $2^n$、$0\le n\le20$ です。法998244353で、各操作は $O((n+1)^2 2^n)$ 時間、$O((n+1)2^n)$ 領域です。

## `set_series_exp`

```text
std::vector<std::uint32_t> set_series_exp(std::span<const std::uint32_t> f);
```

各集合を空でないブロックに分割し、各ブロックの `f` の積を分割方法すべてについて足します。空集合の答えは1です。`f[0]` は法で0を指定します。 係数列はビットマスク順で、長さは $2^n$、$0\le n\le20$ です。法998244353で、各操作は $O((n+1)^2 2^n)$ 時間、$O((n+1)2^n)$ 領域です。

## `set_series_log`

```text
std::vector<std::uint32_t> set_series_log(std::span<const std::uint32_t> f);
```

指数化の逆演算です。`f[0]` は法で1を指定します。戻り値の0番目は0です。 係数列はビットマスク順で、長さは $2^n$、$0\le n\le20$ です。法998244353で、各操作は $O((n+1)^2 2^n)$ 時間、$O((n+1)2^n)$ 領域です。

[使用例](../Examples/set_power_series_example.cpp) ／ [Verify](../Verify/current/submissions/subset_convolution.cpp)
