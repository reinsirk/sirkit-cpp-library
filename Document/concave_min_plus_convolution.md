# 凹数列のmin-plus畳み込み

添字の和ごとに、凹な数列の要素と任意の数列の要素を足したときの最小値を求めます。

```cpp
const std::vector<std::int64_t> a{1, 4, 5}, b{0, 2};
auto c = sirkit::min_plus_convolution_concave_arbitrary(a, b);  // {1, 3, 5, 7}
```

## `min_plus_convolution_concave_arbitrary`

```text
std::vector<std::int64_t> min_plus_convolution_concave_arbitrary(
    std::span<const std::int64_t> a, std::span<const std::int64_t> b);
```

$k=i+j$ となる組のうち、`a[i]+b[j]` の最小値を `c[k]` に返します。`a` の隣接差分は非増加とします。片方が空なら空を返します。和は符号付き64ビットに収め、結果の長さは `INT_MAX` 以下です。$n,m$ を入力の長さとして $O((n+m)\log(n+m+1))$ 時間・$O(n+m)$ 領域です。

[使用例](../Examples/concave_min_plus_convolution_example.cpp) ／ [Verify](../Verify/current/submissions/min_plus_convolution_concave_arbitrary.cpp)
