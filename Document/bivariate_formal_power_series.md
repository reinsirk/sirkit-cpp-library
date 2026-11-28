# 2変数の形式的冪級数の逆数

$x^iy^j$ の係数を $i$ 行 $j$ 列と見て、積が 1 になる係数列を求めます。例えば余分な次数を切りながら、$1/(1-x-y)$ を展開できます。

## `bivariate_fps_inverse`

```text
std::vector<std::uint32_t> bivariate_fps_inverse(
    std::span<const std::uint32_t> coefficients, std::size_t rows, std::size_t columns)
```

$x^iy^j$ の係数は `coefficients[i * columns + j]` です。入出力は `rows * columns` 要素で、積は $2^{21}$ 以下です。係数は 998244353 未満の非負整数とし、定数項は 0 以外を指定します。軸の長さに 0 があれば空の列を返します。$N=\mathrm{rows}\,\mathrm{columns}$ として $O(N\log(N+1))$ 時間・$O(N)$ 領域です。

```cpp
const std::vector<std::uint32_t> coefficients{1, 998244352, 998244352, 0};
const auto inverse = sirkit::bivariate_fps_inverse(coefficients, 2, 2);
// x^0 の行が {1, 1}、x^1 の行が {1, 2}。
```

[使用例](../Examples/bivariate_formal_power_series_example.cpp) ／ [Verify](../Verify/current/submissions/inv_of_formal_power_series_2d.cpp) ／ [同梱NTT](ntt.md)
