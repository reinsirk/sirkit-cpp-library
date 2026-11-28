# 一般グラフの重み付きマッチング

無向グラフで、どの頂点も高々1回使うように辺を選び、重みの合計を最大化します。奇数長の閉路も扱えます。[本体](../Graph/General_Weighted_Matching.cpp)

## `maximum_weight_matching`

```text
GeneralWeightedMatchingResult maximum_weight_matching(
    int n, std::span<const GeneralWeightedMatchingEdge> edges);
```

頂点数 `n` と、辺 `GeneralWeightedMatchingEdge{u, v, weight}` を渡します。頂点番号は `[0,n)` です。自己辺と重みが $0$ 以下の辺を無視し、多重辺は同じ端点間の最大重みを使います。

戻り値の `weight` は最大の重み合計、`edges` は選んだ辺の端点の組です。最大重みが同じ解のうち、選ぶ辺やその個数は規定しません。自己辺を除く正の辺重みは `std::int64_t` の最大値の $1/4$ 以下、解の重みの合計は `std::int64_t` の範囲に収めます。

頂点数 $n$、入力の辺数 $m$ に対し $O(n^3+m)$ 時間、$O(n^2)$ 領域です。

## 使用例

```cpp
const std::array edges{sirkit::GeneralWeightedMatchingEdge{0, 2, 1},
                       sirkit::GeneralWeightedMatchingEdge{1, 3, 1},
                       sirkit::GeneralWeightedMatchingEdge{1, 2, 3}};
const auto answer = sirkit::maximum_weight_matching(4, edges);
// answer.weight == 3、answer.edges == {{1, 2}}
```

[使用例](../Examples/general_weighted_matching_example.cpp)・[Library Checker Verify](../Verify/current/submissions/general_weighted_matching.cpp)
