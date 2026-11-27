# K 最短ウォーク

非負重みの有向グラフで、始点から終点へのウォークの長さを小さい順に取得します。同じ頂点や辺を何度通っても構いません。

```cpp
const std::array edges{
    sirkit::KShortestWalkEdge{0, 1, 2},
    sirkit::KShortestWalkEdge{0, 1, 2},
};
const auto lengths = sirkit::k_shortest_walk_lengths(2, 0, 1, edges, 5);
// lengths == {2, 2}
```

## `k_shortest_walk_lengths`

```text
std::vector<std::int64_t> k_shortest_walk_lengths(
    int n, int source, int target, std::span<const KShortestWalkEdge> edges, std::size_t k);
```

`KShortestWalkEdge{from, to, weight}` で辺を渡します。戻り値はウォークの長さの昇順で、最大 `k` 個です。存在数が `k` に満たなければ、その分だけ返します。

頂点番号は `[0,n)` です。多重辺は別々に数えます。始点と終点が同じときは長さ0の空ウォークも含めます。
重みと戻り値の各要素は `std::int64_t` です。求める長さが範囲を超えると `std::overflow_error` を送出します。

頂点数を $n$、辺数を $m$ とすると、$O(n+m\log(m+1)+k\log(k+1))$ 時間・$O(n+m\log(m+1)+k)$ 領域です。終点への最短路木から外れる辺を永続ヒープで共有します。

[使用例](../Examples/k_shortest_walks_example.cpp) ／ [Verify](../Verify/current/submissions/k_shortest_walk.cpp) ／ [Eppstein (1998)](https://doi.org/10.1137/S0097539795290477)
