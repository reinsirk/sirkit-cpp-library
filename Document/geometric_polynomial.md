# 等比数列上の多項式評価と補間

等比数列上の点をまとめて評価し、値から多項式を復元します。法は998244353です。

```cpp
std::vector<std::uint32_t> f{1, 2, 1}; // (1+x)^2
auto y = sirkit::evaluate_geometric_polynomial(f, 2, 3, 3); // {9, 49, 361}
auto g = sirkit::interpolate_geometric_polynomial(y, 2, 3); // f
```

## `evaluate_geometric_polynomial`

```text
std::vector<std::uint32_t> evaluate_geometric_polynomial(
    std::span<const std::uint32_t> coefficients, std::uint64_t first,
    std::uint64_t common_ratio, std::size_t count);
```

定数項から並べた係数を受け取り、$f(a r^i)$ を $i=0,\ldots,\mathrm{count}-1$ の順で返します。 空の係数列は零多項式、$r^0=1$ です。係数数 $N$ と出力数 $M$ は各 $2^{20}$ 以下。上限を超えると `std::length_error` を送出します。 $O((N+M)\log(N+M+1))$ 時間・$O(N+M+1)$ 領域です。

## `interpolate_geometric_polynomial`

```text
std::optional<std::vector<std::uint32_t>> interpolate_geometric_polynomial(
    std::span<const std::uint32_t> values, std::uint64_t first, std::uint64_t common_ratio);
```

値が $f(a r^i)$ となる次数 $N-1$ 以下の多項式を、定数項から $N$ 個の係数で返します。 $N$ は $2^{20}$ 以下で、上限を超えると `std::length_error` を送出します。評価点が法の上で重複する場合は `std::nullopt`、入力が空なら空の係数列を返します。 $O(N\log(N+1))$ 時間・$O(N+1)$ 領域です。

単独貼り付け版に [NTT](ntt.md) を同梱しています。

参考文献 [Bostan・Schost, Polynomial evaluation and interpolation on special sets of points, §5.2・§5.3](https://mathexp.eu/bostan/publications/BoSc05.pdf)。

[使用例](../Examples/geometric_polynomial_example.cpp) ／ [評価のVerify](../Verify/current/submissions/multipoint_evaluation_on_geometric_sequence.cpp) ／ [補間のVerify](../Verify/current/submissions/polynomial_interpolation_on_geometric_sequence.cpp)
