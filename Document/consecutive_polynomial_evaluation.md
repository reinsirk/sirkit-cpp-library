# 連続する点での多項式評価

値が分かっている連続点から、多項式を別の連続区間で評価します。係数への変換は不要です。

```cpp
const std::vector<std::uint32_t> f{1, 3, 7};  // f(x)=x²+x+1
auto shifted = sirkit::evaluate_consecutive_polynomial(f, 2, 4);  // {7, 13, 21, 31}
```

## `evaluate_consecutive_polynomial`

```text
std::vector<std::uint32_t> evaluate_consecutive_polynomial(
    std::span<const std::uint32_t> samples, std::int64_t first, std::size_t count);
```

`samples[i]` は $f(i)$ です。$n$ 個の評価値を渡すと、次数 $n$ 未満の多項式について `first` から `count` 個の評価値を返します。法は998244353です。`first` が負でも使えます。標本が空なら零多項式として扱います。$n, m=\texttt{count}$ は各 $2^{20}$ 以下で、$O(1+(n+m)\log(n+m+1))$ 時間・$O(1+n+m)$ 領域です。

## 同梱の NTT・畳み込み

`NTT<Mod, Root>` の変換・畳み込み・二乗と、自由関数 `ConvolutionMod` も含みます。[NTTとConvolutionModのAPI](ntt.md#api)

[使用例](../Examples/consecutive_polynomial_evaluation_example.cpp) ／ [Verify](../Verify/current/submissions/shift_of_sampling_points_of_polynomial.cpp)
