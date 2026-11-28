# 階乗 mod 998244353

$n!$ を $998244353$ で割った余りを求めます。$n$ が法以上なら $0$、$0!=1$ です。

## `factorial_mod_998244353`

```text
std::uint32_t factorial_mod_998244353(std::uint64_t n)
```

1 個の階乗を求めます。$n$ が法未満のとき、$M=\min(n,998244352-n)$ として $O(\sqrt{M+1}\log(M+2))$ 時間、$O(\sqrt{M+1})$ 領域です。

## `factorials_mod_998244353`

```text
std::vector<std::uint32_t> factorials_mod_998244353(std::span<const std::uint64_t> queries)
```

途中までの積を共有し、入力と同じ順で答えを返します。空入力なら空です。

クエリ数を $q$ とし、法未満の入力を $\min(n,998244352-n)$ に直した後の最大値を $M$ とします。$M$ が小さいときは $O(M+q\log(q+1))$ 時間です。それ以外では幅 $B$ ごとの積を補間で求めます。$O((B+M/B)\log(B+M/B+1)+q\log(q+1)+\min(M,qB))$ 時間、$O(B+M/B+q)$ 領域です。幅 $B$ は $M$ と $q$ から選びます。

```cpp
const std::array<std::uint64_t, 3> ns{0, 5, 998244352};
const auto answer = sirkit::factorials_mod_998244353(ns); // {1, 120, 998244352}
```

[使用例](../Examples/fast_factorial_example.cpp) ／ [Verify](../Verify/current/submissions/many_factorials.cpp) ／ [連続点での多項式評価](consecutive_polynomial_evaluation.md)
