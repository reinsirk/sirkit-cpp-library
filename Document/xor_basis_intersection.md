# XOR基底の共通部分

整数の部分集合 XOR で作れる値のうち、両方の列から作れるものを基底で表します。

```cpp
const std::array<std::uint64_t, 2> left{3, 5}, right{6, 8};
auto common = sirkit::xor_basis_intersection(left, right); // {6}
```

## `xor_basis_intersection`

```text
std::vector<std::uint64_t> xor_basis_intersection(std::span<const std::uint64_t> left,
                                                  std::span<const std::uint64_t> right);
```

入力は64ビット符号なし整数の列で、0や重複も許します。返り値は両空間の共通部分を張る線形独立なベクトルの列です。共通する値が0だけなら空の基底を返します。

ビット幅を $W=64$、入力の総数を $N$ とすると、$O((N+1)W)$ 時間、$O(W)$ 補助領域です。

[使用例](../Examples/xor_basis_intersection_example.cpp) ／ [Verify](../Verify/current/submissions/intersection_of_f2_vector_spaces.cpp)
