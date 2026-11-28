# 最大二部マッチングの辺と頂点を分類する

`analyze_bipartite_matching(left, right, edges)` は、最大マッチングの大きさと、その大きさを保ったまま使える辺や外せる頂点を返します。両側の頂点番号はそれぞれ $0$ 始まりです。[本体](../Graph/Bipartite_Matching_Analysis.cpp)

- `matching_size` は最大マッチングの辺数です。
- `edge_role[i]` は入力順です。`forbidden` はどの最大マッチングにも入らず、`optional` は入るものと入らないものがあり、`mandatory` はすべてに入ります。並行辺は入力番号ごとに区別します。
- `left_can_be_free[u]` と `right_can_be_free[v]` はその頂点を未対応にできる最大マッチングがあれば真です。

最大マッチングを1回作り、交互閉路と未対応頂点からの交互路を調べます。後処理は全頂点数を $n$、辺数を $m$ として $O(n+m)$ 時間です。全体の領域は $O(n+m)$ です。[二部マッチング](bipartite_matching.md)

## 使用例

```cpp
const std::array edges{std::pair{0, 0}, std::pair{0, 1}, std::pair{1, 1}};
const auto result = sirkit::analyze_bipartite_matching(2, 2, edges);
// 0→1 の辺だけが forbidden。対角の2辺は mandatory。
```

[使用例](../Examples/bipartite_matching_analysis_example.cpp)・[使える辺のVerify](../Verify/current/submissions/bipartite_matching_allowed_edges.cpp)・[必須辺のVerify](../Verify/current/submissions/bipartite_matching_mandatory_edges.cpp)・[未対応頂点のVerify](../Verify/current/submissions/bipartite_matching_free_vertices.cpp)
