# 各点を優越する点の個数

各点について、両方の座標がその点以下である点を数えます。座標ごとに狭義・広義と比較順を指定できます。

## `dominance_counts`

```cpp
template <typename Point, typename XLess = std::less<>, typename YLess = std::less<>>
std::vector<int> dominance_counts(const std::vector<Point> &points,
                                  bool strict_x = false, bool strict_y = false,
                                  XLess x_less = {}, YLess y_less = {});
```

`Point` の `first` と `second` を座標として使います。既定の比較は昇順です。たとえば `x_less` を `std::greater<>{}` にすると、第1座標が大きい点から順に扱います。

`strict_x` または `strict_y` を `true` にすると、その座標が比較順で等しい点を除きます。両方が `false` なら自分自身も数えます。同じ座標の点も、入力にある個数分が対象です。

結果は入力順です。比較関数は狭義弱順序を成し、点数 $n$ は `INT_MAX` 以下とします。比較が $O(1)$ なら $O(1+n\log(n+1))$ 時間・$O(n+1)$ 領域を使います。

## 使用例

```cpp
const std::vector<std::pair<int, int>> points{{1, 2}, {1, 2}, {2, 3}, {3, 1}};
const auto counts = sirkit::dominance_counts(points);
const std::vector<int> expected{2, 2, 3, 1};
assert(counts == expected);
```

[実装](../Algorithm/Dominance_Counts.cpp)・[使用例の全体](../Examples/dominance_counts_example.cpp)・[Verify](../Verify/current/submissions/dominance_counts.cpp)
