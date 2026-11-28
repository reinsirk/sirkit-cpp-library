# 左右の順序を保つ重み付きマッチング

左の列と右の列の間から候補の組を選びます。左の位置で並べたときに右の位置も狭義に増加する、重みの合計が最大の組み合わせを求めます。

## `maximum_weight_monotone_matching`

```text
WeightedMonotoneMatching maximum_weight_monotone_matching(std::span<const WeightedMatchEdge> edges)
```

候補 `WeightedMatchEdge` に左の位置 `left`、右の位置 `right`、重み `weight` を指定します。どちらの位置も同じ値を二度選べません。候補を一つも選ばない場合の重みは0です。

戻り値の `weight` は最大の合計、`indices` は選んだ候補の0始まりの入力添字です。添字は左と右の位置が増加する順で返ります。最適解が複数あればそのうち一つです。右の位置の種類数は `INT_MAX` 以下で、最大和が `int64_t` に収まらなければ `std::overflow_error` が発生します。

候補数を $n$ とすると、$O((n+1)\log(n+2))$ 時間・$O(n+1)$ 領域です。

## 使用例

```cpp
const std::array edges{sirkit::WeightedMatchEdge{0, 2, 8},
                       sirkit::WeightedMatchEdge{1, 1, 6},
                       sirkit::WeightedMatchEdge{2, 2, 9}};
const auto answer = sirkit::maximum_weight_monotone_matching(edges);
assert(answer.weight == 15); // 2番目と3番目を選ぶ
```

[実装](../Algorithm/Weighted_Monotone_Matching.cpp)・[使用例の全体](../Examples/weighted_monotone_matching_example.cpp)
