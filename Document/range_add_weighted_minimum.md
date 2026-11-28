# 区間加算と最小値を取る要素の重み和

値と重みを持つ数列に区間加算を行い、最小値と、その値を取る要素の重み和をまとめて取得します。重みをすべて `1` にすると、最小値を取る要素数が得られます。

| API | 内容 |
| --- | --- |
| `RangeAddWeightedMinimum(values, weights)` | 値と重みの列を指定する。重みだけを渡すと初期値はすべて `0`。$O(n)$ 時間・領域。 |
| `add(l, r, delta)` | 半開区間 `[l,r)` の値に加算。$O(\log(n+1))$。 |
| `minimum(l, r)` / `all_minimum()` | 最小値 `value` と該当する要素の重み和 `weight`。空なら `std::nullopt`。$O(\log(n+1))$ / $O(1)$。 |
| `size()` | 要素数。$O(1)$。 |

値は符号付き整数型で、途中の加算量と値が型の範囲に収まる必要があります。重みは加算できる型を使います。

```cpp
const std::vector<std::uint64_t> weights{2, 3, 7};
sirkit::RangeAddWeightedMinimum<> data(weights);
data.add(1, 2, 4);
const auto result = data.all_minimum().value(); // value = 0, weight = 9
```

[使用例](../Examples/range_add_weighted_minimum_example.cpp) ／ [Verify](../Verify/current/submissions/range_add_weighted_minimum.cpp)
