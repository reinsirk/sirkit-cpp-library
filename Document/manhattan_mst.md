# マンハッタン最小全域木

点同士の距離を `|x1-x2| + |y1-y2|` として、全点をつなぐ最小全域木を求めます。4方向の走査で候補辺を絞り、Kruskal 法で木を選びます。

```cpp
const std::vector<std::pair<std::int64_t, std::int64_t>> points{{0, 0}, {1, 2}, {1, 0}};
const auto tree = sirkit::manhattan_mst(points);
assert(tree.cost == 3);
```

## `manhattan_mst`

```text
ManhattanMSTResult manhattan_mst(std::span<const std::pair<std::int64_t, std::int64_t>> points);
```

`cost` は辺の距離の合計、`edges` は選んだ辺の両端の頂点番号です。番号は入力順の0始まりです。同じ座標の点を入れられます。点が0個か1個なら費用0の空の木を返します。

各座標の絶対値は `INT64_MAX/4` 以下です。費用が `int64_t` に収まらない場合は `std::overflow_error` を送出します。点の数を `N` として、時間は `O(N log(N+1))`、領域は `O(N)` です。

[使用例](../Examples/manhattan_mst_example.cpp) ／ [Verify](../Verify/current/submissions/manhattan_mst.cpp)

参考 [Luzhiled](https://ei1333.github.io/library/graph/mst/manhattan-mst.hpp)
