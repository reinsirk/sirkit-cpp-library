# 固有多項式

正方行列の固有多項式 $\det(xI-A)$ を素数法で求めます。行列には負の要素も渡せます。

```cpp
const std::vector<std::vector<std::int64_t>> a{{1, 2}, {0, 3}};
auto p = sirkit::characteristic_polynomial(a);  // {3, 998244349, 1}
```

## `characteristic_polynomial`

```text
template <std::uint32_t Mod = 998244353>
std::vector<std::uint32_t> characteristic_polynomial(
    std::span<const std::vector<std::int64_t>> matrix);
```

定数項から順に $n+1$ 個の係数を返します。`Mod` は符号なし32ビットで表せる素数を指定します。0行0列の行列の結果は `{1}` です。$O(1+n^3+n\log Mod)$ 時間・$O(1+n^2)$ 領域です。

[使用例](../Examples/characteristic_polynomial_example.cpp) ／ [Verify](../Verify/current/submissions/characteristic_polynomial.cpp)
