# 整数の k 乗根

正の整数 `k` を指定して、`n` の `k` 乗根を切り捨てた整数を求めます。浮動小数点で得た近似を出発点にし、完全べき乗の前後も整数演算で判定します。

```cpp
assert(sirkit::integer_kth_root(215, 3) == 5);
assert(sirkit::integer_kth_root(216, 3) == 6);
```

## `integer_kth_root`

```text
std::uint64_t integer_kth_root(std::uint64_t n, unsigned k);
```

`x^k ≤ n` を満たす最大の非負整数 `x` を返します。`k ≥ 1` を指定します。`n` は `std::uint64_t` の全域を扱えます。

最悪時間は `O(1 + ceil(log(n+2)/k) log(k+1))` です。追加領域は `O(1)` です。

[使用例](../Examples/integer_root_example.cpp) ／ [Verify](../Verify/current/submissions/kth_root_integer.cpp) ／ [Luzhiled の関連実装](https://ei1333.github.io/library/math/number-theory/kth-root-integer.hpp.html)
