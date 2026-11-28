# 部分列の最小値・最大値

連続部分列の極値を担当する添字を求めたり、長さごとに極値を合計したりできます。`before` の既定値は `std::less<>` で、最小値を使います。最大値を使う場合は `std::greater<>{}` を渡します。

| API | 戻り値 |
|---|---|
| `subarray_extremum_spans(values, before)` | 各添字 $i$ の範囲 `[left,right)`。この範囲に収まり $i$ を含む部分列では、$i$ が極値を担当します。同値の極値は左側を優先します。 |
| `subarray_extremum_sums_by_length<Sum>(values, before)` | 長さ $k$ の部分列すべての極値の和を、添字 $k-1$ に置きます。 |

`values` は `std::vector` や `std::span` など、長さが分かるランダムアクセス範囲です。`Sum{}` は0で、各値を `Sum` に変換でき、途中の加減算が有効であることが前提です。比較・加減算が $O(1)$ なら、それぞれ $O(n)$ 時間・領域です。空列では空列を返します。

```cpp
const std::vector<int> a{3, 1, 2};
const auto sums = sirkit::subarray_extremum_sums_by_length<std::int64_t>(a);
// 長さ1, 2, 3の順で {6, 2, 1}。
```

[使用例](../Examples/subarray_extrema_example.cpp) ／ [Verify](../Verify/current/submissions/subarray_extrema_agc005_b.cpp)
