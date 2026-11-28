# 成功確率が異なる独立試行

試行ごとの成功確率を与え、成功回数の分布や、指定回数以上成功する確率を求めます。成功回数はポアソン二項分布に従います。

入力には長さを取得できる浮動小数点の列を渡し、各確率を0以上1以下で指定します。

## `poisson_binomial_distribution(probabilities)`

試行が $n$ 回なら、添字 `k` が成功 $k$ 回の確率を表す長さ $n+1$ の配列を返します。$O(1+n^2)$ 時間、$O(n+1)$ 領域です。

## `poisson_binomial_at_least(probabilities, k)`

成功回数が $k$ 以上になる確率です。$k=0$ なら1、$k>n$ なら0を返します。$1\le k\le n$ のとき、$O(n\min(k,n-k+1))$ 時間、$O(1+\min(k,n-k+1))$ 領域です。

成功数か失敗数の少ない範囲だけを計算し、確率は非負の項の和で求めます。返り値には入力と同じ浮動小数点型を使います。

## 使用例

```cpp
const std::array probabilities{0.30, 0.60, 0.80};
const auto chance = sirkit::poisson_binomial_at_least(probabilities, 2);
assert(std::abs(chance - 0.612) < 1e-12);
```

[実装](../Math/Poisson_Binomial.cpp)・[使用例の全体](../Examples/poisson_binomial_example.cpp)・[Verify](../Verify/current/submissions/poisson_binomial.cpp)
