# 多項式列の積

複数の多項式をまとめて掛け、法998244353の係数列を求めます。

```cpp
auto result = sirkit::polynomial_sequence_product({{1, 1}, {1, 1}, {3}});
// {3, 6, 3}
```

## `polynomial_sequence_product`

```text
std::vector<std::uint32_t> polynomial_sequence_product(
    std::vector<std::vector<std::uint32_t>> factors);
```

各係数列は定数項から並べます。零多項式は空の係数列で表し、空積は `{1}` です。元の列が不要なら `std::move(factors)` で領域を渡せます。 短い因子から掛け、定数はまとめて係数に掛けます。総係数数 $C$ と総次数 $D$ について $O(C+D\log^2(D+2))$ 時間・$O(C+1)$ 領域です。 零でない積の $D+1$ が $2^{23}$ を超えると `std::length_error` を送出します。

単独貼り付け版に [NTT](ntt.md) を同梱しています。

[使用例](../Examples/polynomial_sequence_product_example.cpp) ／ [Verify](../Verify/current/submissions/product_of_polynomial_sequence.cpp)
