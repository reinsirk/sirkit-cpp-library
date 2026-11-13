# アルゴリズムの説明

各文書は概要・API・短い使用例・重要な前提を中心にまとめます。完全な使用例は`Examples/`、関数一覧はソース先頭、検証・測定記録は`Audit/`を参照してください。

## グラフ

- [Dijkstra](dijkstra.md)
- [Floyd–Warshall](floyd_warshall.md)
- [最近共通祖先](lca.md)
- [トポロジカルソート](topological.md)
- [負辺を含む最短路](negative_shortest_paths.md)
- [Project Selection](project_selection.md)
- [強連結成分分解](scc.md)
- [2-SAT](two_sat.md)
- [重軽分解](heavy_light_decomposition.md)
- [最大流](max_flow.md)
- [Low-linkと二重連結成分](low_link.md)
- [オイラー路](eulerian_trail.md)
- [最小費用流](min_cost_flow.md)
- [Prüfer符号と指定次数の木](pruefer_code.md)
- [重心分解](centroid_decomposition.md)
- [二部グラフの最大マッチング](bipartite_matching.md)
- [隣接順を保つ全方位木DP](rerooting_dp.md)
- [OfflineDynamicConnectivity](offline_dynamic_connectivity.md)
- [FunctionalGraph](functional_graph.md)
- [CartesianTree](cartesian_tree.md)
- [GeneralMatching](general_matching.md)

## データ構造

- [動的Fenwick Tree](dynamic_fenwick.md)
- [動的2次元Fenwick Tree](dynamic_fenwick_2d.md)
- [動的Li Chao Tree](dynamic_li_chao.md)
- [動的Union Find](dynamic_union_find.md)
- [削除可能な優先度付きキュー](erasable_priority_queue.md)
- [HashMap](hash_map.md)
- [HashSet](hash_set.md)
- [最小ヒープ](heap.md)
- [暗黙Treap](implicit_treap.md)
- [2次元LineContainer](line_container_2d.md)
- [LineContainer](line_container.md)
- [永続Union Findと永続配列](persistent_union_find.md)
- [巻き戻しUnion Find](rollback_union_find.md)
- [重み付き巻き戻しUnion Find](rollback_weighted_union_find.md)
- [Union Find](union_find.md)
- [重み付きUnion Find](weighted_union_find.md)
- [二分トライと圧縮二分トライ](binary_trie.md)
- [単方向ノードと連結列](node.md)
- [Disjoint Sparse Table](disjoint_sparse_table.md)
- [Fenwick Tree](fenwick_tree.md)
- [Lazy Segment Tree](lazy_segment_tree.md)
- [Segment Tree](segment_tree.md)
- [Sparse Table](sparse_table.md)
- [Wavelet Matrix](wavelet_matrix.md)
- [永続セグメント木](persistent_segment_tree.md)
- [Segment Tree Beats](segment_tree_beats.md)
- [Sliding Window Aggregation](sliding_window_aggregation.md)
- [Link Cut Tree](link_cut_tree.md)
- [双対セグメント木](dual_segment_tree.md)
- [Slope Trick](slope_trick.md)
- [DynamicBitset](dynamic_bitset.md)

## 数学

- [二項係数](binomial_coefficient.md)
- [分数](fraction.md)
- [ゼータ／メビウス変換と整数篩](zeta_mobius.md)
- [エラトステネスの篩](prime_sieve.md)
- [64ビット素数判定](primality.md)
- [NTTと素数法畳み込み](ntt.md)
- [64ビット整数の因数分解](pollard_rho.md)
- [形式的冪級数と多項式](formal_power_series.md)
- [XOR基底](xor_basis.md)
- [線形漸化式](linear_recurrence.md)
- [行列と体上の消去](matrix.md)
- [BinaryMatrix](binary_matrix.md)
- [任意法・整数畳み込み](convolution.md)
- [多項式の多点評価・補間・Taylor shift](polynomial.md)
- [64ビットの剰余演算・合同式・平方根](modular_arithmetic.md)

## 文字列

- [Rolling Hash](rolling_hash.md)
- [Trie](trie.md)
- [Z配列と文字列検索](z_algorithm.md)
- [回文半径](manacher.md)
- [接尾辞配列](suffix_array.md)
- [Aho–Corasick法](aho_corasick.md)
- [Suffix Automaton](suffix_automaton.md)
- [回文木](palindromic_tree.md)

## 幾何

- [凸包](convex_hull.md)
- [最近点対](nearest_two_points.md)
- [円の交差](geometry_circle_intersection.md)
- [点の内外判定](geometry_point_location.md)
- [方向判定](geometry_orientation.md)
- [点と線分の距離](geometry_point_segment_distance.md)
- [線分交差](geometry_segment_intersection.md)
- [面積](geometry_area.md)
- [点と座標演算](geometry_point.md)
- [偏角ソート](angle_sort.md)

## その他のアルゴリズム

- [最長増加部分列](lis.md)
- [行列変換](matrix_transform.md)
- [組合せマスク](combination_masks.md)
- [Radix Sort](radix_sort.md)
- [Mo / MoWithUpdates](mo.md)

## 共通・テンプレート

- [SIMDによる内部処理](simd.md)
- [内部整数ハッシュ表](internal_hash_map.md)
- [コンテスト用テンプレート](template.md)
- [コンパイラ別版と速度優先版](fast_variants.md)
- [ライブラリの連結と容量確認](bundling.md)
