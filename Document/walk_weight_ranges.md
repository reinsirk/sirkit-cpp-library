# 有向グラフの歩道の重みの範囲

始点から各頂点までの歩道について、辺重みの総和の最小値と最大値を求めます。頂点も辺も再訪でき、始点での空の歩道も含みます。[本体](../Graph/Walk_Weight_Ranges.cpp)

- `walk_weight_ranges(n, edges, source)` は、頂点番号を添字とする結果を返します。辺は `WalkWeightEdge{from, to, weight}` で、端点は 0 始まり、重みは符号付き 32 ビットです。時間・領域は $O(n+m)$ です。
- 各要素の `reachable` は到達可否です。最小・最大の一方以上に限りがなければ `unbounded` が真になります。到達可能で有界な頂点では`minimum`・`maximum` が符号付き 64 ビットの値です。

強連結成分の中で辺重みを頂点間のポテンシャル差として表せれば、成分内を何周しても重みの総和は変わりません。表せない成分を通ると、非零の閉じた歩道を繰り返せます。成分を縮約して、到達可否と重みの範囲を伝えます。

## 使用例

```cpp
const std::array edges{sirkit::WalkWeightEdge{0, 1, -3}, sirkit::WalkWeightEdge{0, 1, 5},
                       sirkit::WalkWeightEdge{1, 2, 2}, sirkit::WalkWeightEdge{2, 2, 1}};
const auto range = sirkit::walk_weight_ranges(4, edges, 0);
// range[1] の範囲は [-3, 5]。range[2] は非有界、range[3] は到達不能。
```

[使用例](../Examples/walk_weight_ranges_example.cpp)・[Verify](../Verify/current/submissions/walk_weight_ranges.cpp)
