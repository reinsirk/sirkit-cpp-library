# 有向最小全域木

根から全頂点に到達できるように有向辺を選び、その重みの合計を最小化します。

```cpp
const auto tree = sirkit::minimum_directed_spanning_tree(n, root, edges);
if (tree)
{
    auto total = tree->cost;
    auto chosen = tree->incoming_edge[v];  // v != root
    auto parent = edges[chosen].from;
}
```

## `minimum_directed_spanning_tree`

```text
std::optional<DirectedMstResult> minimum_directed_spanning_tree(
    int n, int root, std::span<const DirectedMstEdge> edges);
```

- `edges` に `DirectedMstEdge{from, to, weight}` の列を渡します。頂点数 $n$ は1以上、根と端点の番号は `[0,n)` です。負の重み、自己辺、多重辺も扱えます。
- 戻り値は `std::optional<DirectedMstResult>` です。全頂点に届く木がなければ `std::nullopt` を返します。
- `cost` は符号付き64ビットの費用、`incoming_edge[v]` は頂点 `v` に入る辺の入力添字です。根の欄は `DirectedMstResult::kNoEdge` です。最小費用が64ビット符号付き整数の範囲を超えると `std::overflow_error` を送出します。

辺数を $m$ とすると、$O(n+m\log(m+1))$ 時間・$O(n+m)$ 領域です。多重辺のないグラフでは $O(n+m\log(n+1))$ 時間です。

[使用例](../Examples/minimum_directed_spanning_tree_example.cpp) ／ [Verify](../Verify/current/submissions/directedmst.cpp) ／ [Tarjan (1977)](https://doi.org/10.1002/net.3230070103)
