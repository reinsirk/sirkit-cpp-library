# グラフ

各ライブラリは必要な標準ヘッダーを含み、単独で貼り付けられます。

| ファイル | 対応する検証 |
| --- | --- |
| [Bellman-Ford.cpp](Bellman-Ford.cpp) | [E - Coins Respawn](../Verify/current/submissions/bellman_ford.cpp) |
| [Bellman-Ford_SFPA.cpp](Bellman-Ford_SFPA.cpp) | [E - Coins Respawn](../Verify/current/submissions/spfa.cpp) |
| [Dijkstra.cpp](Dijkstra.cpp) | [Shortest Path](../Verify/current/submissions/dijkstra.cpp) |
| [Floyd-Warshall.cpp](Floyd-Warshall.cpp) | [D - バスと避けられない運命](../Verify/current/submissions/floyd_warshall.cpp) |
| [LCA.cpp](LCA.cpp) | [Lowest Common Ancestor](../Verify/current/submissions/lca.cpp) |
| [Project_Selection.cpp](Project_Selection.cpp) | [040 - Get More Money（★7）](../Verify/current/submissions/project_selection.cpp) |
| [Topological.cpp](Topological.cpp) | [G - Longest Path](../Verify/current/submissions/topological.cpp) |
| [Topological_sort.cpp](Topological_sort.cpp) | [D - Restricted Permutation](../Verify/current/submissions/topological_lexicographical.cpp) |
| [Low_Link.cpp](Low_Link.cpp) | [Two-Edge-Connected Components](../Verify/current/submissions/low_link_two_edge.cpp)・[Biconnected Components](../Verify/current/submissions/low_link_biconnected.cpp) |
| [Eulerian_Trail.cpp](Eulerian_Trail.cpp) | [Eulerian Trail (Directed)](../Verify/current/submissions/eulerian_trail_directed.cpp)・[Eulerian Trail (Undirected)](../Verify/current/submissions/eulerian_trail_undirected.cpp) |
| [Min_Cost_Flow.cpp](Min_Cost_Flow.cpp) | [E - MinCostFlow](../Verify/current/submissions/min_cost_flow_grid.cpp) |
| [Pruefer_Code.cpp](Pruefer_Code.cpp) | [Construct Highway](../Verify/current/submissions/pruefer_code.cpp) |
| [Centroid_Decomposition.cpp](Centroid_Decomposition.cpp) | [Balanced Tree](../Verify/current/submissions/centroid_decomposition.cpp) |
| [Bipartite_Matching.cpp](Bipartite_Matching.cpp) | [Bipartite Matching](../Verify/current/submissions/bipartite_matching.cpp) |
| [Rerooting_DP.cpp](Rerooting_DP.cpp) | [Tree Path Composite Sum](../Verify/current/submissions/rerooting_dp.cpp) |
| [Offline_Dynamic_Connectivity.cpp](Offline_Dynamic_Connectivity.cpp) | [Dynamic Graph Vertex Add Component Sum](../Verify/current/submissions/offline_dynamic_connectivity.cpp) |
| [Functional_Graph.cpp](Functional_Graph.cpp) | [E - Permute K times](../Verify/current/submissions/functional_graph.cpp) |
| [Cartesian_Tree.cpp](Cartesian_Tree.cpp) | [Cartesian Tree](../Verify/current/submissions/cartesian_tree.cpp) |
| [General_Matching.cpp](General_Matching.cpp) | [Matching on General Graph](../Verify/current/submissions/general_matching.cpp) |

問題URL、追加APIの補足検査、現在版の判定状況は[Verify一覧](../Verify/current/README.md)を参照してください。

## Dijkstra

[アルゴリズムと前提](../Document/dijkstra.md)・[使用例](../Examples/dijkstra_example.cpp)を参照してください。
重みは`[0, 1e9]`です。経路付きの呼び出しでも全頂点の最短距離を計算します。
現在のソースと判定の対応は[Verifyのmanifest](../Verify/current/manifest.json)で確認できます。

## LCA

[アルゴリズムと前提](../Document/lca.md)・[使用例](../Examples/lca_example.cpp)を参照してください。

[トポロジカルソートの説明](../Document/topological.md)に通常順と辞書順の前提・使用例をまとめています。

[Bellman–FordとSPFAの説明](../Document/negative_shortest_paths.md)に負閉路判定・未到達値・入力の前提をまとめています。

[Project Selectionの説明](../Document/project_selection.md)に費用・利得の表現、解の復元、再利用の前提をまとめています。

## 連結成分・木・フロー

| ファイル | 対応する検証 |
| --- | --- |
| [SCC.cpp](SCC.cpp) | [Strongly Connected Components](../Verify/current/submissions/scc.cpp) |
| [Two_SAT.cpp](Two_SAT.cpp) | [2 Sat](../Verify/current/submissions/two_sat.cpp) |
| [Heavy_Light_Decomposition.cpp](Heavy_Light_Decomposition.cpp) | [Jump on Tree](../Verify/current/submissions/hld_jump.cpp)・[Vertex Add Path Sum](../Verify/current/submissions/hld_vertex_add_path_sum.cpp) |
| [Max_Flow.cpp](Max_Flow.cpp) | [D - Maxflow](../Verify/current/submissions/max_flow_grid.cpp) |
