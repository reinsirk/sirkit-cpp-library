# 任意の法での行列式

整数行列の行列式を、指定した法で求めます。合成数の法にも対応します。

```cpp
std::vector<std::vector<std::int64_t>> a{{2, 1}, {3, 4}};
assert(sirkit::determinant_mod(a, 6) == 5);
```

## `determinant_mod`

```text
std::uint32_t determinant_mod(std::span<const std::vector<std::int64_t>> matrix, std::uint32_t mod);
```

正方行列と正の法を指定します。行列式の最小非負剰余を返します。空行列の行列式は1です。行と列の長さを $n$ として $O(n^3\log(mod+1))$ 時間・$O(n^2)$ 領域です。

[使用例](../Examples/determinant_arbitrary_mod_example.cpp) ／ [Verify](../Verify/current/submissions/matrix_det_arbitrary_mod.cpp)
