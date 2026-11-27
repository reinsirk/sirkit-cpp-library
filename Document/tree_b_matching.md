# 次数上限付きで木の辺を選ぶ

各頂点の上限を守って木の辺を選び、選んだ辺の重みの総和を最大にします。上限をすべて1にすると、木の最大重みマッチングになります。

```cpp
const std::array cap{1, 1, 1};
const std::array edges{sirkit::TreeBMatchingEdge{0, 1, 4},
                       sirkit::TreeBMatchingEdge{1, 2, 7}};
const auto best = sirkit::maximum_weight_tree_b_matching(cap, edges);
// best.weight == 7、best.edge_ids == {1}。
```

## `maximum_weight_tree_b_matching`

```text
TreeBMatchingResult maximum_weight_tree_b_matching(
    std::span<const int> max_degree, std::span<const TreeBMatchingEdge> edges);
```

頂点番号は0始まりです。`max_degree[v]` に非負の次数上限、`TreeBMatchingEdge{from, to, weight}` に無向辺と符号付き64ビットの重みを渡します。

`weight` は最大の重みの和、`edge_ids` は選んだ辺の0始まりの入力添字です。複数の最適解があればそのうち1つを返し、辺を選ばない解も認めます。空木と1頂点は重み0・辺列は空。最適値が `std::int64_t` の範囲外なら `std::overflow_error` を送出します。

頂点数を $n$ として $O(1+n\log(n+1))$ 時間・$O(1+n)$ 領域です。

[使用例](../Examples/tree_b_matching_example.cpp) ／ [Verify](../Verify/current/submissions/tree_b_matching.cpp) ／ [AtCoderの解説](https://atcoder.jp/contests/abc259/editorial/4296)
