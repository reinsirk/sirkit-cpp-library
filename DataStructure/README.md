# データ構造

各ライブラリは必要な標準ヘッダーを含み、単独で貼り付けられます。

`HashMap`と`HashSet`の既定のハッシュ関数は、利用者も直接使える`GenericHash`です。
内部専用のハッシュ表・比較関数・循環検査は`sirkit::detail`に置きます。
HashMapの削除とiteratorの計算量は[HashMapの説明](../Document/hash_map.md)を参照してください。[使用例](../Examples/hash_map_example.cpp)もあります。
可変長列の集約・遅延作用・所有の扱いは[暗黙Treapの説明](../Document/implicit_treap.md)を参照してください。
点の追加と内積の最適化は[2次元LineContainerの説明](../Document/line_container_2d.md)と[使用例](../Examples/line_container_2d_example.cpp)を参照してください。
直線の追加と最小値・最大値の照会は[LineContainerの説明](../Document/line_container.md)と[使用例](../Examples/line_container_example.cpp)を参照してください。

| ファイル | 対応する検証 |
| --- | --- |
| [Binary_Trie.cpp](Binary_Trie.cpp) | [Set Xor-Min](../Verify/current/submissions/binary_trie.cpp) |
| [Compressed_Binary_Trie.cpp](Compressed_Binary_Trie.cpp) | [Set Xor-Min（圧縮版）](../Verify/current/submissions/compressed_binary_trie.cpp) |
| [Hash_map.cpp](Hash_map.cpp) | [Associative Array](../Verify/current/submissions/hash_map.cpp) |
| [Hash_set.cpp](Hash_set.cpp) | [整数追加・検索](../Verify/current/submissions/hash_set_membership.cpp)・[追加・削除](../Verify/current/submissions/hash_set_toggle.cpp)・[文字列](../Verify/current/submissions/hash_set_strings.cpp) |
| [Node.cpp](Node.cpp) | [Takahashi's Secret](../Verify/current/submissions/node_reachable.cpp)・[独立ローカル検査](../Verify/current/submissions/node_local.cpp) |
| [dynamic-binary-indexed-tree-2d.cpp](dynamic-binary-indexed-tree-2d.cpp) | [Point Add Rectangle Sum](../Verify/current/submissions/dynamic_fenwick_2d_rectangle.cpp) |
| [dynamic-binary-indexed-tree.cpp](dynamic-binary-indexed-tree.cpp) | [Point Add Range Sum](../Verify/current/submissions/dynamic_fenwick_range.cpp) |
| [dynamic-li-chao-tree.cpp](dynamic-li-chao-tree.cpp) | [Segment Add Get Min](../Verify/current/submissions/dynamic_lichao_segment.cpp) |
| [dynamic-union-find.cpp](dynamic-union-find.cpp) | [Disjoint Set: Union Find Tree](../Verify/current/submissions/dynamic_union_find.cpp) |
| [erasable-priority-queue.cpp](erasable-priority-queue.cpp) | [Double-Ended Priority Queue](../Verify/current/submissions/erasable_priority_queue_double.cpp) |
| [heap.cpp](heap.cpp) | [Prefix K-th Max](../Verify/current/submissions/heap_priority_queue.cpp)・[説明](../Document/heap.md) |
| [implicit_treap.cpp](implicit_treap.cpp) | [区間アフィン作用・和](../Verify/current/submissions/implicit_treap_affine.cpp)・[両端操作・関数合成](../Verify/current/submissions/implicit_treap_deque.cpp)・[ID指定の挿入・区間削除](../Verify/current/submissions/implicit_treap_erase_between.cpp) |
| [line-container-2d.cpp](line-container-2d.cpp) | [Line Add Get Min](../Verify/current/submissions/line_container_2d_min.cpp) |
| [line-container.cpp](line-container.cpp) | [Line Add Get Min](../Verify/current/submissions/line_container_min.cpp) |
| [persistent-union-find.cpp](persistent-union-find.cpp) | [Persistent Unionfind](../Verify/current/submissions/persistent_union_find.cpp) |
| [rollback-union-find.cpp](rollback-union-find.cpp) | [Persistent Unionfind](../Verify/current/submissions/rollback_union_find.cpp) |
| [rollback-union-findw.cpp](rollback-union-findw.cpp) | [Unionfind with Potential](../Verify/current/submissions/rollback_weighted_union_find.cpp) |
| [union_find.cpp](union_find.cpp) | [Unionfind](../Verify/current/submissions/union_find.cpp) |
| [union_find_w.cpp](union_find_w.cpp) | [Unionfind with Potential](../Verify/current/submissions/weighted_union_find.cpp) |
| [Persistent_Segment_Tree.cpp](Persistent_Segment_Tree.cpp) | [Point Add Range Sum](../Verify/current/submissions/persistent_segment_tree.cpp) |
| [Segment_Tree_Beats.cpp](Segment_Tree_Beats.cpp) | [Range Chmin Chmax Add Range Sum](../Verify/current/submissions/segment_tree_beats.cpp) |
| [Sliding_Window_Aggregation.cpp](Sliding_Window_Aggregation.cpp) | [Queue Operate All Composite](../Verify/current/submissions/sliding_window_aggregation.cpp) |
| [Link_Cut_Tree.cpp](Link_Cut_Tree.cpp) | [Dynamic Tree Vertex Add Path Sum](../Verify/current/submissions/link_cut_tree.cpp)・[Dynamic Tree Vertex Set Path Composite](../Verify/current/submissions/link_cut_tree_composite.cpp)・[Range Affine Range Sum](../Verify/current/submissions/link_cut_tree_affine.cpp) |
| [Dual_Segment_Tree.cpp](Dual_Segment_Tree.cpp) | [Range Affine Point Get](../Verify/current/submissions/dual_segment_tree.cpp) |
| [Slope_Trick.cpp](Slope_Trick.cpp) | [Absolute Minima](../Verify/current/submissions/slope_trick.cpp) |
| [Dynamic_Bitset.cpp](Dynamic_Bitset.cpp) | [Matrix Product (Mod 2)](../Verify/current/submissions/dynamic_bitset.cpp)・[G - Triangle](../Verify/current/submissions/dynamic_bitset_triangle.cpp)・[A - コンテスト](../Verify/current/submissions/dynamic_bitset_shift_or.cpp) |
| [Double_Ended_Priority_Queue.cpp](Double_Ended_Priority_Queue.cpp) | [Double-Ended Priority Queue](../Verify/current/submissions/double_ended_priority_queue.cpp) |
| [Range_Majority.cpp](Range_Majority.cpp) | [Majority Voting](../Verify/current/submissions/majority_voting.cpp) |
| [Offline_Rectangle_Add_Point_Get.cpp](Offline_Rectangle_Add_Point_Get.cpp) | [Rectangle Add Point Get](../Verify/current/submissions/rectangle_add_point_get.cpp) |
| [Point_Set_Range_Frequency.cpp](Point_Set_Range_Frequency.cpp) | [Point Set Range Frequency](../Verify/current/submissions/point_set_range_frequency.cpp) |
| [Static_Rectangle_Add_Rectangle_Sum.cpp](Static_Rectangle_Add_Rectangle_Sum.cpp) | [Static Rectangle Add Rectangle Sum](../Verify/current/submissions/static_rectangle_add_rectangle_sum.cpp) |
| [Persistent_Queue.cpp](Persistent_Queue.cpp) | [Persistent Queue](../Verify/current/submissions/persistent_queue.cpp) |
| [Range_Parallel_Union_Find.cpp](Range_Parallel_Union_Find.cpp) | [Range Parallel Unionfind](../Verify/current/submissions/range_parallel_unionfind.cpp) |
| [Static_Range_Mode.cpp](Static_Range_Mode.cpp) | [Static Range Mode Query](../Verify/current/submissions/static_range_mode_query.cpp) |
| [Range_Linear_Add_Range_Min.cpp](Range_Linear_Add_Range_Min.cpp) | [Range Linear Add Range Min](../Verify/current/submissions/range_linear_add_range_min.cpp) |
| [Range_Sort_Aggregation.cpp](Range_Sort_Aggregation.cpp) | [Point Set Range Sort Range Composite](../Verify/current/submissions/point_set_range_sort_range_composite.cpp) |
| [Noncommutative_Weighted_Union_Find.cpp](Noncommutative_Weighted_Union_Find.cpp) | [Unionfind with Potential (Non-Commutative Group)](../Verify/current/submissions/unionfind_with_potential_non_commutative_group.cpp) |
| [Offline_Point_Add_Rectangle_Sum.cpp](Offline_Point_Add_Rectangle_Sum.cpp) | [Point Add Rectangle Sum](../Verify/current/submissions/offline_point_add_rectangle_sum.cpp) |
| [Point_Rectangle_Affine_KD_Tree.cpp](Point_Rectangle_Affine_KD_Tree.cpp) | [Dynamic Point Set Rectangle Affine Rectangle Sum](../Verify/current/submissions/dynamic_point_set_rectangle_affine_rectangle_sum.cpp) |
| [Sparse_Affine_Range_Sum.cpp](Sparse_Affine_Range_Sum.cpp) | [Range Affine Range Sum (Large Array)](../Verify/current/submissions/range_affine_range_sum_large_array.cpp) |
| [Weighted_Wavelet_Matrix.cpp](Weighted_Wavelet_Matrix.cpp) | [Static Range Sum with Upper Bound](../Verify/current/submissions/static_range_sum_with_upper_bound.cpp) |
| [Persistent_Affine_Range_Sum.cpp](Persistent_Affine_Range_Sum.cpp) | [Persistent Range Affine Range Sum](../Verify/current/submissions/persistent_range_affine_range_sum.cpp) |
| [Sparse_Segment_Tree.cpp](Sparse_Segment_Tree.cpp) | [Point Set Range Composite (Large Array)](../Verify/current/submissions/point_set_range_composite_large_array.cpp) |
| [Range_Assign_Range_Product.cpp](Range_Assign_Range_Product.cpp) | [Range Set Range Composite](../Verify/current/submissions/range_set_range_composite.cpp) |
| [Deque_Aggregation.cpp](Deque_Aggregation.cpp) | [Deque Operate All Composite](../Verify/current/submissions/deque_operate_all_composite.cpp) |
| [Euler_Tour_Tree.cpp](Euler_Tour_Tree.cpp) | [Dynamic Tree Vertex Add Subtree Sum](../Verify/current/submissions/dynamic_tree_vertex_add_subtree_sum.cpp)・[Dynamic Tree Subtree Add Subtree Sum](../Verify/current/submissions/dynamic_tree_subtree_add_subtree_sum.cpp) |
| [Hierarchical_Bitset.cpp](Hierarchical_Bitset.cpp) | [Predecessor Problem](../Verify/current/submissions/predecessor_problem.cpp) |

問題URL、追加APIの補足検査、現在版の判定状況は[Verify一覧](../Verify/current/README.md)を参照してください。

## 二分トライの選択

`BinaryTrie`はビットごとのトライ、`CompressedBinaryTrie`は分岐しない経路を省く版です。どちらも重複・ID・順位照会・遅延XORを扱えます。
`insert_unique(x,id=-1)`は未登録時だけ追加してtrueを返し、`reserve(n)`で領域を予約できます。
方式・APIの前提は[説明](../Document/binary_trie.md)、使用例は[通常版](../Examples/binary_trie_example.cpp)・[圧縮版](../Examples/compressed_binary_trie_example.cpp)を参照してください。

## 動的BIT

[1次元](../Document/dynamic_fenwick.md)・[2次元](../Document/dynamic_fenwick_2d.md)の説明と、[1次元](../Examples/dynamic_fenwick_example.cpp)・[2次元](../Examples/dynamic_fenwick_2d_example.cpp)の使用例を参照してください。
現在の提出コードと判定の対応は[Verifyのmanifest](../Verify/current/manifest.json)で確認できます。

永続Union Findと永続配列の版・所有権・2点更新は[説明](../Document/persistent_union_find.md)と[使用例](../Examples/persistent_union_find_example.cpp)を参照してください。

巻き戻しUnion Findの履歴・snapshot・履歴消去は[説明](../Document/rollback_union_find.md)と[使用例](../Examples/rollback_union_find_example.cpp)を参照してください。

重み付き巻き戻しUnion Findの差・整合性・整数範囲は[説明](../Document/rollback_weighted_union_find.md)と[使用例](../Examples/rollback_weighted_union_find_example.cpp)を参照してください。

Union Findの代表・経路圧縮・初期化は[説明](../Document/union_find.md)と[使用例](../Examples/union_find_example.cpp)を参照してください。

重み付きUnion Findの差・整合性・型の前提は[説明](../Document/weighted_union_find.md)と[使用例](../Examples/union_find_w_example.cpp)を参照してください。

## 区間の集約

| ファイル | 対応する検証 |
| --- | --- |
| [Fenwick_Tree.cpp](Fenwick_Tree.cpp) | [Point Add Range Sum](../Verify/current/submissions/fenwick_tree.cpp)・[Vertex Add Path Sum](../Verify/current/submissions/hld_vertex_add_path_sum.cpp) |
| [Segment_Tree.cpp](Segment_Tree.cpp) | [Point Add Range Sum](../Verify/current/submissions/segment_tree.cpp)・[Point Set Range Composite](../Verify/current/submissions/segment_tree_composite.cpp) |
| [Lazy_Segment_Tree.cpp](Lazy_Segment_Tree.cpp) | [Range Affine Range Sum](../Verify/current/submissions/lazy_segment_tree.cpp) |
| [Sparse_Table.cpp](Sparse_Table.cpp) | [Static RMQ](../Verify/current/submissions/sparse_table.cpp) |
| [Disjoint_Sparse_Table.cpp](Disjoint_Sparse_Table.cpp) | [Static RMQ](../Verify/current/submissions/disjoint_sparse_table.cpp) |
| [Wavelet_Matrix.cpp](Wavelet_Matrix.cpp) | [Range Kth Smallest](../Verify/current/submissions/wavelet_kth.cpp)・[Static Range Frequency](../Verify/current/submissions/wavelet_frequency.cpp) |
| [Dynamic_Wavelet_Matrix.cpp](Dynamic_Wavelet_Matrix.cpp) | [Point Set Range Frequency（動的ウェーブレット行列）](../Verify/current/submissions/dynamic_wavelet_matrix_frequency.cpp)<br>[ABC241 D - Sequence Query（動的ウェーブレット行列）](../Verify/current/submissions/dynamic_wavelet_matrix_order_statistics.cpp) |
| [Indexed_Priority_Queue.cpp](Indexed_Priority_Queue.cpp) | [Shortest Path（ID付き優先度付きキュー）](../Verify/current/submissions/indexed_priority_queue_shortest_path.cpp) |
| [Interval_Set.cpp](Interval_Set.cpp) | [Range Insert & Point Mex](../Verify/current/submissions/interval_set_range_mex.cpp) |
| [Persistent_Stack.cpp](Persistent_Stack.cpp) | [Notebook](../Verify/current/submissions/persistent_stack_notebook.cpp) |
| [Radix_Heap.cpp](Radix_Heap.cpp) | [Shortest Path（単調最小キュー）](../Verify/current/submissions/radix_heap_shortest_path.cpp) |
| [Segment_Tree_2D.cpp](Segment_Tree_2D.cpp) | [School of Killifish](../Verify/current/submissions/segment_tree_2d_rectangle_minimum.cpp) |
| [Sparse_Table_2D.cpp](Sparse_Table_2D.cpp) | [Stamp Game](../Verify/current/submissions/sparse_table_2d_stamps.cpp) |
| [Top_K_Sum.cpp](Top_K_Sum.cpp) | [Least Elements](../Verify/current/submissions/top_k_sum_sliding.cpp)<br>[Best Performances](../Verify/current/submissions/top_k_sum_updates.cpp) |
| [Range_Fenwick_Tree.cpp](Range_Fenwick_Tree.cpp) | [AWC0014 E - Flowerbed Watering Management](../Verify/current/submissions/range_fenwick_tree.cpp) |
| [Static_Range_Minimum.cpp](Static_Range_Minimum.cpp) | [Static RMQ（線形構築・線形領域）](../Verify/current/submissions/static_range_minimum_linear.cpp) |
| [Compressed_Fenwick_Tree_2D.cpp](Compressed_Fenwick_Tree_2D.cpp) | [Point Add Rectangle Sum（座標圧縮Fenwick木）](../Verify/current/submissions/compressed_fenwick_tree_2d.cpp) |
| [Persistent_Binary_Trie.cpp](Persistent_Binary_Trie.cpp) | [Range Kth Smallest（永続二分Trie）](../Verify/current/submissions/persistent_binary_trie_range_kth.cpp)<br>[Set Xor-Min（永続二分Trie）](../Verify/current/submissions/persistent_binary_trie_xor_min.cpp) |
| [Persistent_Li_Chao_Tree.cpp](Persistent_Li_Chao_Tree.cpp) | [Line Add Get Min（永続Li Chao木）](../Verify/current/submissions/persistent_li_chao_line.cpp)<br>[Segment Add Get Min（永続Li Chao木）](../Verify/current/submissions/persistent_li_chao_segment.cpp) |
| [Union_Find_Order_Statistics.cpp](Union_Find_Order_Statistics.cpp) | [ABC372 E - K-th Largest Connected Components](../Verify/current/submissions/union_find_order_statistics_rank.cpp)<br>[ABC183 F - Confluence](../Verify/current/submissions/union_find_order_statistics_count.cpp) |
| [Offline_Li_Chao_Tree.cpp](Offline_Li_Chao_Tree.cpp) | [いろはちゃんコンテスト Day4 L - ...好きです](../Verify/current/submissions/offline_li_chao_tree_coins.cpp) |
| [Monotone_Line_Container.cpp](Monotone_Line_Container.cpp) | [Educational DP Contest Z - Frog 3](../Verify/current/submissions/monotone_line_container_frog.cpp) |
| [Meldable_Heap.cpp](Meldable_Heap.cpp) | [Game on Tree 3](../Verify/current/submissions/meldable_heap_tree_game.cpp) |
| [Persistent_Weighted_Union_Find.cpp](Persistent_Weighted_Union_Find.cpp) | [AWC0100 M - 秘密の数列と分岐するノート](../Verify/current/submissions/persistent_weighted_union_find_branching.cpp) |
| [Point_Set_Mex.cpp](Point_Set_Mex.cpp) | [Mex and Update](../Verify/current/submissions/point_set_mex.cpp)<br>[Range Insert & Point Mex](../Verify/current/submissions/point_set_mex_ranges.cpp) |
| [Persistent_Affine_Sequence.cpp](Persistent_Affine_Sequence.cpp) | [Persistent Range Affine Range Sum（永続アフィン数列）](../Verify/current/submissions/persistent_affine_sequence.cpp)<br>[ARC030 D - グラフではない](../Verify/current/submissions/persistent_affine_sequence_range_copy.cpp) |
| [Sliding_Window_Extrema.cpp](Sliding_Window_Extrema.cpp) | [Sliding Minimum Element](../Verify/current/submissions/sliding_window_extrema.cpp)<br>[Stamp Game](../Verify/current/submissions/sliding_window_extrema_stamps.cpp) |
| [Rollback_Lazy_Segment_Tree.cpp](Rollback_Lazy_Segment_Tree.cpp) | [yukicoder No.3671 - Reusable Lazy Segment Tree](../Verify/current/submissions/rollback_lazy_segment_tree.cpp) |
| [Interval_Map.cpp](Interval_Map.cpp) | [1D Bucket Tool](../Verify/current/submissions/interval_map_paint.cpp)<br>[Range Update Query (RUQ)](../Verify/current/submissions/interval_map_range_assign.cpp) |
| [Ring_Deque.cpp](Ring_Deque.cpp) | [Deque](../Verify/current/submissions/ring_deque.cpp) |
| [Range_Add_Range_Min.cpp](Range_Add_Range_Min.cpp) | [Range Add Range Min](../Verify/current/submissions/range_add_range_min.cpp) |
| [Binary_Range_Statistics.cpp](Binary_Range_Statistics.cpp) | [F - Vacation Query](../Verify/current/submissions/binary_range_statistics.cpp) |
| [Range_Equal_Runs.cpp](Range_Equal_Runs.cpp) | [Run](../Verify/current/submissions/range_equal_runs.cpp) |
| [Range_Nondecreasing_Subsequence.cpp](Range_Nondecreasing_Subsequence.cpp) | [Range ABCD String Query](../Verify/current/submissions/range_nondecreasing_subsequence.cpp) |
| [Range_Add_Range_Gcd.cpp](Range_Add_Range_Gcd.cpp) | [D - ARCたんクッキー](../Verify/current/submissions/range_add_range_gcd.cpp) |
| [Range_Add_Threshold_Min.cpp](Range_Add_Threshold_Min.cpp) | [Choosing Blocks in a Shopping Street](../Verify/current/submissions/range_add_threshold_min.cpp) |
| [Range_Add_State_Ids.cpp](Range_Add_State_Ids.cpp) | [yukicoder No.469 - 区間加算と一致検索の問題](../Verify/current/submissions/range_add_state_ids.cpp) |
| [Component_Add_Union_Find.cpp](Component_Add_Union_Find.cpp) | [Union add query](../Verify/current/submissions/component_add_union_find.cpp) |
| [Range_Add_Weighted_Minimum.cpp](Range_Add_Weighted_Minimum.cpp) | [N - Times Tables](../Verify/current/submissions/range_add_weighted_minimum.cpp) |
| [Successor_Set.cpp](Successor_Set.cpp) | [D - Linear Probing](../Verify/current/submissions/successor_set_linear_probing.cpp) |
| [Sqrt_Tree.cpp](Sqrt_Tree.cpp) | [Static RMQ](../Verify/current/submissions/sqrt_tree_static_rmq.cpp) |
| [Monotone_Value_Shift.cpp](Monotone_Value_Shift.cpp) | [Rated Range](../Verify/current/submissions/monotone_value_shift.cpp) |
| [Nearest_Value_Index.cpp](Nearest_Value_Index.cpp) | [K-th Nearest](../Verify/current/submissions/nearest_value_index.cpp) |
| [Grid_Connectivity_Frontier.cpp](Grid_Connectivity_Frontier.cpp) | [S - マス目](../Verify/current/submissions/grid_corner_connected_colorings.cpp) |
| [Run_Length_Queue.cpp](Run_Length_Queue.cpp) | [Large Queue](../Verify/current/submissions/run_length_queue.cpp) |
| [Weighted_Slope_Trick.cpp](Weighted_Slope_Trick.cpp) | [No.1467 Selling Cars](../Verify/current/submissions/weighted_slope_trick_selling_cars.cpp) |
| [Weighted_L1_Statistics.cpp](Weighted_L1_Statistics.cpp) | [F - Absolute Minima](../Verify/current/submissions/weighted_l1_statistics_absolute_minima.cpp)<br>[D - Chalkboard Median](../Verify/current/submissions/weighted_l1_statistics_chalkboard.cpp) |
| [Ordered_Multiset.cpp](Ordered_Multiset.cpp) | [Max - Min Query](../Verify/current/submissions/ordered_multiset_extrema.cpp)<br>[Sequence Query](../Verify/current/submissions/ordered_multiset_ranks.cpp) |
| [Range_Add_Total_Variation.cpp](Range_Add_Total_Variation.cpp) | [Uplift](../Verify/current/submissions/range_add_total_variation.cpp) |
| [Subset_Xor_Square_Range.cpp](Subset_Xor_Square_Range.cpp) | [XOR^2 Sum Query](../Verify/current/submissions/subset_xor_square_range_yuki3792.cpp) |
