# 床関数の和

`floor_sum(n, m, a, b)` は、`i = 0, …, n-1` について `floor((a*i+b)/m)` を足した値を返します。例えば、各行の長さが一次式で変わる格子点の個数を数えられます。

```cpp
assert(sirkit::floor_sum(4, 3, 2, 1) == 4); // 0 + 1 + 1 + 2
assert(sirkit::floor_sum(2, 3, -1, 0) == -1); // 0 + (-1)
assert(sirkit::min_of_mod_of_linear(4, 7, 2, 3) == 0); // 3, 5, 0, 2
```

## `floor_sum`

```text
std::int64_t floor_sum(std::int64_t n, std::int64_t m,
                       std::int64_t a, std::int64_t b);
```

`0 ≤ n < 2^32`、`1 ≤ m < 2^32`。負の `a`、`b` も指定でき、各項は負の無限大へ丸めます。和は `2^64` を法とし、符号付き64ビット整数で返します。時間は `O(log(m+1))`、追加領域は `O(1)` です。

## `min_of_mod_of_linear`

```text
std::int64_t min_of_mod_of_linear(std::int64_t n, std::int64_t m,
                                  std::int64_t a, std::int64_t b);
```

`i = 0, …, n-1` での `(a*i+b) mod m` の最小値を返します。余りは0以上 `m` 未満です。`1 ≤ n < 2^32`、`1 ≤ m < 2^32` で、`a` と `b` は負でも構いません。時間は `O(log(m+1))`、追加領域は `O(1)` です。

[使用例](../Examples/floor_sum_example.cpp) ／ [和のVerify](../Verify/current/submissions/floor_sum.cpp) ／ [最小値のVerify](../Verify/current/submissions/min_of_mod_of_linear.cpp)

参考 [AC Library の仕様](https://atcoder.github.io/ac-library/production/document_ja/math.html) ／ [Nyaan](https://nyaannyaan.github.io/library/math/floor-sum.hpp)
