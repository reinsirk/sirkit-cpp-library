# トーティエント関数の総和

1 から $n$ までのオイラーのトーティエント関数の値を足し、指定した法での余りを求めます。

```cpp
auto result = sirkit::summatory_totient(5);  // 10
```

## `summatory_totient`

```text
std::uint32_t summatory_totient(std::uint64_t n,
                               std::uint32_t mod = 998244353);
```

$\sum_{i=1}^n\varphi(i)\bmod{mod}$ を返します。合成数の法も使えます。$n=0$ のときは $0$、法 $0$ は `std::invalid_argument`。 小さい範囲を線形篩で前計算し、大きい範囲は商が等しい区間をまとめます。$O(n^{2/3})$ 時間・$O(n^{2/3})$ 領域です。 篩の添字が32ビットを超えると `std::length_error` を投げます。

[使用例](../Examples/summatory_totient_example.cpp) ／ [Verify](../Verify/current/submissions/sum_of_totient_function.cpp)
