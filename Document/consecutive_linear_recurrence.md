# 線形漸化式の連続項

線形漸化式で定まる数列から、指定した位置の連続項を法998244353で求めます。

```cpp
const std::array<std::uint32_t, 2> initial{0, 1}, coefficients{1, 1};
auto terms = sirkit::consecutive_linear_recurrence(initial, coefficients, 5, 4);
// フィボナッチ数列の第5項から {5, 8, 13, 21}。
```

## `consecutive_linear_recurrence`

```text
std::vector<std::uint32_t> consecutive_linear_recurrence(
    std::span<const std::uint32_t> initial, std::span<const std::uint32_t> coefficients,
    std::uint64_t first, std::size_t count);
```

両入力の長さを $d$ とし、$i\ge d$ で $a_i=\sum_{j=0}^{d-1}\mathtt{coefficients}[j]\, a_{i-j-1}$ とします。返すのは0始まりの第 `first` 項から昇順に `count` 項です。入力の係数を法で正規化します。`count=0` なら空列、$d=0$ なら0だけの列を返します。

通常版で入力の長さが異なると `std::invalid_argument`、最後の添字が `std::uint64_t` を超えると `std::overflow_error` です。`count>0` で $d+\mathtt{count}>2^{22}$ のときは `std::length_error` を送出します。

母関数の分母を偶奇で分け、必要な連続係数をまとめて取り出します。$k=\mathtt{first}, M=\mathtt{count}$ として $O(d\log(d+1)\log(k+2)+(d+M)\log(d+M+1))$ 時間、$O(d\log(k+2)+M)$ 領域です。 [同梱NTTの使い方](ntt.md)

[使用例](../Examples/consecutive_linear_recurrence_example.cpp) ／ [Verify](../Verify/current/submissions/consecutive_terms_of_linear_recurrent_sequence.cpp) ／ [Bostan–Moriの原論文](https://specfun.inria.fr/bostan/BoMo21.pdf)
