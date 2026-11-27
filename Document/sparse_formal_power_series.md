# 疎な形式的冪級数

法998244353の形式的冪級数を、`SparseFpsTerm{degree, coefficient}` の列で渡します。`degree` は `std::size_t`、`coefficient` は `std::uint32_t` です。入力順は任意で、同じ次数の係数は法で足し合わせます。

```cpp
const std::array terms{sirkit::SparseFpsTerm{0, 1}, sirkit::SparseFpsTerm{1, 998244352}};
auto inverse = sirkit::sparse_fps_inverse(4, terms);  // 1/(1-x) は {1, 1, 1, 1}
```

結果は次数0から並べた長さ `n` の係数列です。次数が `n` 以上の入力項は無視し、`n=0` なら空列を返します。逆元を除き `n` は法以下とし、超えると `std::length_error` です。入力を $K$ 項とすると、各操作は $O(K\log(K+1)+n(K+1))$ 時間、$O(n+K)$ 領域です。非零項が多い入力には密な係数列の演算も使えます。 [形式的冪級数](formal_power_series.md)

## `sparse_fps_inverse`

```text
std::vector<std::uint32_t> sparse_fps_inverse(std::size_t n, std::span<const SparseFpsTerm> input);
```

逆元 $1/f$ を求めます。定数項は非零とし、通常版でこの条件を満たさない場合は `std::invalid_argument` を送出します。

## `sparse_fps_exp`

```text
std::vector<std::uint32_t> sparse_fps_exp(std::size_t n, std::span<const SparseFpsTerm> input);
```

形式指数 $\exp(f)$ を求めます。定数項は0とし、通常版でこの条件を満たさない場合は `std::invalid_argument` です。

## `sparse_fps_log`

```text
std::vector<std::uint32_t> sparse_fps_log(std::size_t n, std::span<const SparseFpsTerm> input);
```

形式対数 $\log(f)$ を求めます。定数項は1とし、通常版でこの条件を満たさない場合は `std::invalid_argument` です。

## `sparse_fps_pow`

```text
std::vector<std::uint32_t> sparse_fps_pow(
    std::size_t n, std::span<const SparseFpsTerm> input, std::uint64_t exponent);
```

非負整数 `exponent` 乗を求めます。指数が0なら入力によらず級数1を返します。

## `sparse_fps_sqrt`

```text
std::optional<std::vector<std::uint32_t>> sparse_fps_sqrt(
    std::size_t n, std::span<const SparseFpsTerm> input);
```

平方が入力と $x^n$ 未満で一致する級数を返します。存在しなければ `std::nullopt` です。根の先頭にある非零係数は符号のうち小さい剰余を選び、高次の不定な係数は0にします。`n=0` の戻り値は空の係数列を含みます。

[使用例](../Examples/sparse_formal_power_series_example.cpp) ／ [逆元のVerify](../Verify/current/submissions/inv_of_formal_power_series_sparse.cpp) ／ [指数のVerify](../Verify/current/submissions/exp_of_formal_power_series_sparse.cpp) ／ [対数のVerify](../Verify/current/submissions/log_of_formal_power_series_sparse.cpp) ／ [整数乗のVerify](../Verify/current/submissions/pow_of_formal_power_series_sparse.cpp) ／ [平方根のVerify](../Verify/current/submissions/sqrt_of_formal_power_series_sparse.cpp)
