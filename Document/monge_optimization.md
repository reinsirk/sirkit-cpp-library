# 行最小値探索と min-plus 畳み込み

行ごとに最良の列を探すとき、最良列が右へしか動かないなら、すべての要素を調べずに済みます。行列の値は比較関数から必要なときだけ計算できます。

```cpp
const auto columns = sirkit::smawk(3, 4, [](int row, int current, int candidate)
{
    return std::abs(row - candidate) < std::abs(row - current);
}); // {0, 1, 2}
```

`better(row, current, candidate)` は候補が厳密に良いとき `true` を返します。同点では最左列を選びます。列数が0なら各行の答えは `-1` です。

## `monotone_minima`

```text
template <class Compare>
std::vector<int> monotone_minima(int height, int width, Compare better);
```

元の行列の最良列が非減少なら使えます。時間は `O(height+width log(height+1))`、領域は `O(height)` です。

## `smawk`

```text
template <class Compare>
std::vector<int> smawk(int height, int width, Compare better);
```

任意の部分行列でも最良列が非減少なら使えます。時間と領域は `O(height+width)` です。

## `min_plus_convolution_convex_arbitrary`

```text
template <class T>
std::vector<T> min_plus_convolution_convex_arbitrary(const std::vector<T> &a, const std::vector<T> &b);
```

出力の `k` 番目は `i+j=k` を満たす `a[i]+b[j]` の最小値です。`a` は隣接差分が非減少である必要があります。和と大小比較を丸めずに行い、計算する和が `T` に収まる必要があります。片方が空なら空列を返します。時間と領域は `O(|a|+|b|)` です。

## `min_plus_convolution_convex_convex`

```text
template <class T>
std::vector<T> min_plus_convolution_convex_convex(const std::vector<T> &a, const std::vector<T> &b);
```

両方の隣接差分が非減少のとき、同じ min-plus 畳み込みを求めます。時間は `O(|a|+|b|)`、出力以外の領域は `O(1)` です。和と空入力の扱いは前の関数と共通です。

[使用例](../Examples/monge_optimization_example.cpp) ／ [凸・任意のVerify](../Verify/current/submissions/min_plus_convex_arbitrary.cpp) ／ [凸・凸のVerify](../Verify/current/submissions/min_plus_convex_convex.cpp)

参考 [Luzhiled の SMAWK](https://ei1333.github.io/library/dp/smawk.hpp) ／ [AtCoder Algorithm Lectures](https://info.atcoder.jp/entry/algorithm_lectures/min_plus_convolution)
