# 区間加算・区間GCD

数列に区間加算を行い、現在の区間の最大公約数を求めます。添字は0始まりで、区間は半開です。

| API | 内容 |
| --- | --- |
| `RangeAddRangeGcd(a)` | 符号付き整数の列から構築する。 |
| `size()` / `get(i)` | 要素数と現在の値。$O(1)$ / $O(\log(n+1))$。 |
| `add(l, r, delta)` | `[l,r)` の全要素に加算する。 |
| `gcd(l, r)` | `[l,r)` の最大公約数。空区間の値は `0`。 |

加算後の各値は元の型 `T` に収まる必要があります。たとえば `INT64_MIN` 一つの区間は最大公約数が $2^{63}$ なので、`gcd` は `T` に対応する符号なし型を返します。

値と階差の絶対値の上限を $M$ とすると、構築は $O(n(1+\log(M+1)))$ 時間・$O(n)$ 領域です。`add` と `gcd` は $O(\log(n+1)(1+\log(M+1)))$ 時間です。

```cpp
const std::vector<long long> a{6, 9, 15};
sirkit::RangeAddRangeGcd data(a);
data.add(1, 3, 3);         // {6, 12, 18}
const auto d = data.gcd(0, 3); // 6
```

[使用例](../Examples/range_add_range_gcd_example.cpp) ／ [Verify](../Verify/current/submissions/range_add_range_gcd.cpp)
