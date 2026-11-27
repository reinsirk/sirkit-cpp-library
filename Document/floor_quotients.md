# 整数除算の商の列挙

`n/i` を整数除算した商は、異なる `i` で同じになることがあります。商が変わる位置だけをたどると、商の種類や、商ごとにまとめた和を `O(√(n+1))` 時間で求められます。

```cpp
const auto values = sirkit::enumerate_quotients(10); // {1, 2, 3, 5, 10}
```

## `for_each_floor_quotient`

```text
template <class Visit>
void for_each_floor_quotient(std::uint64_t n, Visit &&visit);
```

`1 ≤ i ≤ n` を商が一定の閉区間に分け、`i` の昇順に `visit(first, last, quotient)` を呼びます。`n = 0` では呼びません。時間は各呼び出しを除いて `O(√(n+1))`、追加領域は `O(1)` です。

## `enumerate_quotients`

```text
std::vector<std::uint64_t> enumerate_quotients(std::uint64_t n);
```

商を重複なしの昇順で返します。`n = 0` のときは空列です。時間と領域は `O(√(n+1))` です。

[使用例](../Examples/floor_quotients_example.cpp) ／ [Verify](../Verify/current/submissions/enumerate_quotients.cpp) ／ [Nyaan の関連実装](https://nyaannyaan.github.io/library/math/enumerate-quotient.hpp)
