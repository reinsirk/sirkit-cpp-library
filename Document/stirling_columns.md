# スターリング数の列

$k$ を固定し、$n=k$ から指定した上限までのスターリング数をまとめて返します。答えは法 $998244353$ での剰余です。

## `stirling_first_column`

```text
std::vector<std::uint32_t> stirling_first_column(std::size_t n, std::size_t k)
```

符号付き第1種 $s(k,k),\ldots,s(n,k)$ を返します。$s(i,k)$ は下降階乗 $x(x-1)\cdots(x-i+1)$ の $x^k$ の係数です。

## `stirling_second_column`

```text
std::vector<std::uint32_t> stirling_second_column(std::size_t n, std::size_t k)
```

第2種 $S(k,k),\ldots,S(n,k)$ を返します。$S(i,k)$ は $i$ 個の要素を $k$ 個の空でない集合に分ける個数です。

どちらも $k>n$ なら空です。$n<998244353$ かつ列の長さ $L=n-k+1\leq2^{22}$ が必要です。$O(L\log(L+1)+\log(k+1))$ 時間、$O(L)$ 領域です。

```cpp
const auto s = sirkit::stirling_first_column(4, 2);  // {1, 998244350, 11}
const auto S = sirkit::stirling_second_column(4, 2); // {1, 3, 7}
```

[使用例](../Examples/stirling_columns_example.cpp) ／ [第1種のVerify](../Verify/current/submissions/stirling_number_of_the_first_kind_fixed_k.cpp) ／ [第2種のVerify](../Verify/current/submissions/stirling_number_of_the_second_kind_fixed_k.cpp) ／ [同梱FPS](formal_power_series.md)
