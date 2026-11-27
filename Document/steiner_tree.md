# 最小シュタイナー木

非負の重みを持つ無向グラフで、選んだ頂点をすべて結ぶ木のうち、重みの合計が最小のものを求めます。指定していない頂点も経由でき、結ぶ頂点が少ない場合に使えます。

```cpp
const std::array<sirkit::SteinerTreeEdge, 3> edges{{{0, 1, 1}, {1, 2, 2}, {0, 2, 7}}};
const std::array terminals{0, 2};
const auto tree = sirkit::minimum_steiner_tree(3, edges, terminals);
assert(tree && tree->cost == 3);
```

## `minimum_steiner_tree`

```text
std::optional<SteinerTree> minimum_steiner_tree(
    int n, std::span<const SteinerTreeEdge> edges, std::span<const int> terminals);
```

頂点番号は `[0,n)`、辺は `SteinerTreeEdge{from, to, weight}` の順に指定します。重みは非負の `std::int64_t` です。自己辺と多重辺を扱えます。

戻り値の `cost` が最小費用で、`edge_ids` は選んだ辺の入力位置です。全指定頂点を結べない場合は `std::nullopt` を返します。重複を除いた指定頂点が0個または1個の場合、答えは費用0と空の辺列です。最小費用が `std::int64_t` に収まらない場合は `std::overflow_error` を送出します。費用が型の上限と等しい解も扱えます。

頂点数を $n$、辺数を $m$、指定頂点の入力数を $T$、異なる指定頂点数を $k$ とします。$k\ge2$ のとき $b=k-1$ として $O(T+3^b n+2^b(n+m)\log(n+m+1))$ 時間・$O(2^b n+m)$ 領域です。部分集合を表すビットマスクと表が格納可能な規模で使います。$k\le1$ のとき $O(n+T)$ 時間・$O(n)$ 領域です。

[使用例](../Examples/steiner_tree_example.cpp) ／ [Verify](../Verify/current/submissions/minimum_steiner_tree.cpp) ／ [Dreyfus–Wagnerの原論文](https://doi.org/10.1002/net.3230010302)
