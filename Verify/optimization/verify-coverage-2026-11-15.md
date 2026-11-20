# Verify先と検証範囲の再調査（2026-11-15）

優先順位は **Library Checker → yukicoder → AtCoder → AOJ**。1ライブラリに複数問題を用意し、問題名だけで全API検証済みとしません。
調査開始時点のLC130本のうち、99本は対象APIに直接対応し、31本は部分的な検証に適していました。対象APIを使わないダミー提出はありませんでした。未検証の操作・型・状態は[130本の監査記録](../../Audit/verify-coverage-final-2026-11-15.json)に記載しています。

## 主な不足と補完

| 対象 | 従来の不足 | 今回の補完 |
| --- | --- | --- |
| Binary/Compressed Trie | 匿名集合の最小XORだけ | 順位・前後要素、全体XORとID保持/削除 |
| Persistent Segment Tree | 最新版だけの区間和 | 過去版から分岐する永続キュー（過去版の範囲prodはローカル補足） |
| Disjoint Sparse Table | minだけ | 非可換・非冪等の関数合成 |
| Fenwick/Segment Tree | 更新・和中心 | lower/upper_bound、max_right/min_left |
| 最大流 | 容量1の二部マッチング | 一般容量とflow_limitのYuki177 |
| LineContainer2D | 方向(x,1)だけ | 三角形面積と任意方向の線形最大化 |
| Mo | 左右対称の操作だけ | 左右別のcallbackを使う転倒数 |
| Set/Sparse Transform | 下位集合操作なし、疎なのに連続キー | 下位集合2変換、穴のある約数閉包 |
| AhoCorasick/RollingHash | 構造だけ、同一個体lcpだけ | 本文総一致数、底27の短区間get/whole |
| 重み付きrollback UF | LCの重みとrollbackは別々の問題 | 既存Yuki2293・AWC0100 Mも併用（非零重みのrollbackを検証） |

中心APIの全入力や全overloadをこれだけで保証しません。とくにハッシュ衝突・検査/例外・所有権・別の型・未使用操作は、独立参照のローカル試験と区別して確認します。

## 今回追加した提出コード（39本）

すべてオンライン未提出です。前回用意した[13本の移行用コード](verify-targets-2026-11-15.md)とは別の追加分です。旧ACは転記していません。

| ソース | 提出先 | 検証対象 |
| --- | --- | --- |
| [coverage_mo_inversions.cpp](../current/submissions/coverage_mo_inversions.cpp) | LC: [Static Range Inversions Query（左右非対称Mo）](https://judge.yosupo.jp/problem/static_range_inversions_query) | Mo.runのadd_left/add_right/erase_left/erase_rightを別々に使い、各位置の転倒数を集計する |
| [coverage_next_combination_independent_set.cpp](../current/submissions/coverage_next_combination_independent_set.cpp) | LC: [Maximum Independent Set（組合せ列挙）](https://judge.yosupo.jp/problem/maximum_independent_set) | for_each_combination/first_combination/next_combination、k=0から全個数、32bitマスク |
| [coverage_sparse_gcd_holes_full.cpp](../current/submissions/coverage_sparse_gcd_holes_full.cpp) | LC: [GCD Convolution（穴のある疎なキー）](https://judge.yosupo.jp/problem/gcd_convolution) | sparse_multiple_zeta_transformとsparse_multiple_mobius_transform、非連続・全約数閉包のキー |
| [coverage_sparse_gcd_holes_split.cpp](../current/submissions/coverage_sparse_gcd_holes_split.cpp) | LC: [GCD Convolution（穴のある疎なキー）](https://judge.yosupo.jp/problem/gcd_convolution) | sparse_multiple_zeta_transformとsparse_multiple_mobius_transform、非連続・全約数閉包のキー |
| [coverage_structure_binary_ordered.cpp](../current/submissions/coverage_structure_binary_ordered.cpp) | LC: [Ordered Set (BinaryTrie)](https://judge.yosupo.jp/problem/ordered_set) | insert_unique/del・重複なしget_kth/count_less・順位・個数・前後要素 |
| [coverage_structure_binary_xor_ids.cpp](../current/submissions/coverage_structure_binary_xor_ids.cpp) | LC: [Set Xor-Min (global XOR / IDs, BinaryTrie)](https://judge.yosupo.jp/problem/set_xor_min) | operate_xor後のinsert_unique/find/指定ID付きdel・min_elementのID一覧を用いた答え復元 |
| [coverage_structure_compressed_ordered.cpp](../current/submissions/coverage_structure_compressed_ordered.cpp) | LC: [Ordered Set (CompressedBinaryTrie)](https://judge.yosupo.jp/problem/ordered_set) | insert_unique/del・重複なしget_kth/count_less・順位・個数・前後要素 |
| [coverage_structure_compressed_xor_ids.cpp](../current/submissions/coverage_structure_compressed_xor_ids.cpp) | LC: [Set Xor-Min (global XOR / IDs, CompressedBinaryTrie)](https://judge.yosupo.jp/problem/set_xor_min) | operate_xor後のinsert_unique/find/指定ID付きdel・min_elementのID一覧を用いた答え復元 |
| [coverage_structure_disjoint_composite.cpp](../current/submissions/coverage_structure_disjoint_composite.cpp) | LC: [Queue Operate All Composite (DisjointSparseTable)](https://judge.yosupo.jp/problem/queue_operate_all_composite) | 非冪等・非可換なprod・空区間の単位元・O(QlogQ)事前構築とO(1)照会 |
| [coverage_structure_fenwick_predecessor.cpp](../current/submissions/coverage_structure_fenwick_predecessor.cpp) | LC: [Predecessor Problem (fenwick boundary search)](https://judge.yosupo.jp/problem/predecessor_problem) | set/get・非負0/1列のlower_bound/upper_bound・sumの直後/直前の境界 |
| [coverage_structure_persistent_queue.cpp](../current/submissions/coverage_structure_persistent_queue.cpp) | LC: [Persistent Queue (PersistentSegmentTree)](https://judge.yosupo.jp/problem/persistent_queue) | 不変Rootの保持・任意の旧版からset分岐・過去版のget・同じ位置の分岐上書き独立性 |
| [coverage_structure_segment_predecessor.cpp](../current/submissions/coverage_structure_segment_predecessor.cpp) | LC: [Predecessor Problem (segment boundary search)](https://judge.yosupo.jp/problem/predecessor_problem) | set/get・max_right/min_leftの述語境界探索・0/1計数モノイド |
| [coverage_subset_transform_full.cpp](../current/submissions/coverage_subset_transform_full.cpp) | LC: [Bitwise And Convolution（部分集合変換）](https://judge.yosupo.jp/problem/bitwise_and_convolution) | subset_zeta_transformとsubset_mobius_transform、0次元からの全部分集合和 |
| [coverage_subset_transform_split.cpp](../current/submissions/coverage_subset_transform_split.cpp) | LC: [Bitwise And Convolution（部分集合変換）](https://judge.yosupo.jp/problem/bitwise_and_convolution) | subset_zeta_transformとsubset_mobius_transform、0次元からの全部分集合和 |
| [coverage_structure_line2d_linear.cpp](../current/submissions/coverage_structure_line2d_linear.cpp) | AtCoder: [Linear Maximization (LineContainer2D)](https://atcoder.jp/contests/abc244/tasks/abc244_h) | 任意の整数方向でmax_rounded_ll・係数の両符号・零・軸方向・原点・座標/係数±1e9 |
| [coverage_aho_total_search.cpp](../current/submissions/coverage_aho_total_search.cpp) | yukicoder: [文字列検索（AhoCorasick総一致数）](https://yukicoder.me/problems/no/430) | AhoCorasick<26,'A'>のadd/build/count_total、重なる一致とfailure遷移 |
| [coverage_geometry_circle_in_diamond_full.cpp](../current/submissions/coverage_geometry_circle_in_diamond_full.cpp) | yukicoder: [UFOを捕まえろ](https://yukicoder.me/problems/no/172) | contains（中心のInside判定）・distance(Point,Segment)（境界の接触除外） |
| [coverage_geometry_circle_in_diamond_split.cpp](../current/submissions/coverage_geometry_circle_in_diamond_split.cpp) | yukicoder: [UFOを捕まえろ](https://yukicoder.me/problems/no/172) | contains（中心のInside判定）・distance(Point,Segment)（境界の接触除外） |
| [coverage_geometry_coin_overlap_full.cpp](../current/submissions/coverage_geometry_coin_overlap_full.cpp) | yukicoder: [1円玉投げ](https://yukicoder.me/problems/no/202) | circle_intersection_kind（None/Tangent/TwoPoints/Coincident） |
| [coverage_geometry_coin_overlap_split.cpp](../current/submissions/coverage_geometry_coin_overlap_split.cpp) | yukicoder: [1円玉投げ](https://yukicoder.me/problems/no/202) | circle_intersection_kind（None/Tangent/TwoPoints/Coincident） |
| [coverage_geometry_convex_polygon_area_full.cpp](../current/submissions/coverage_geometry_convex_polygon_area_full.cpp) | yukicoder: [Greedy MtSaka](https://yukicoder.me/problems/no/1447) | area（整数座標の凸多角形） |
| [coverage_geometry_convex_polygon_area_split.cpp](../current/submissions/coverage_geometry_convex_polygon_area_split.cpp) | yukicoder: [Greedy MtSaka](https://yukicoder.me/problems/no/1447) | area（整数座標の凸多角形） |
| [coverage_geometry_nearest_sorted_line.cpp](../current/submissions/coverage_geometry_nearest_sorted_line.cpp) | yukicoder: [距離（整列済み最近点対）](https://yukicoder.me/problems/no/716) | nearest_two_points_sorted(vector<P>)・NearestTwoPointsResult.distance_squared |
| [coverage_geometry_orientation_stabbing_full.cpp](../current/submissions/coverage_geometry_orientation_stabbing_full.cpp) | yukicoder: [貫け！（向き判定）](https://yukicoder.me/problems/no/245) | orientation（整数・共線・異なる向き） |
| [coverage_geometry_orientation_stabbing_split.cpp](../current/submissions/coverage_geometry_orientation_stabbing_split.cpp) | yukicoder: [貫け！（向き判定）](https://yukicoder.me/problems/no/245) | orientation（整数・共線・異なる向き） |
| [coverage_geometry_parabola_polygon_area_full.cpp](../current/submissions/coverage_geometry_parabola_polygon_area_full.cpp) | yukicoder: [面積の求め方（多角形近似）](https://yukicoder.me/problems/no/1009) | area（long double・8193頂点・退化図形） |
| [coverage_geometry_parabola_polygon_area_split.cpp](../current/submissions/coverage_geometry_parabola_polygon_area_split.cpp) | yukicoder: [面積の求め方（多角形近似）](https://yukicoder.me/problems/no/1009) | area（long double・8193頂点・退化図形） |
| [coverage_geometry_segment_stabbing_full.cpp](../current/submissions/coverage_geometry_segment_stabbing_full.cpp) | yukicoder: [貫け！（線分交差）](https://yukicoder.me/problems/no/245) | intersects（端点接触・共線重なりを含む閉線分） |
| [coverage_geometry_segment_stabbing_split.cpp](../current/submissions/coverage_geometry_segment_stabbing_split.cpp) | yukicoder: [貫け！（線分交差）](https://yukicoder.me/problems/no/245) | intersects（端点接触・共線重なりを含む閉線分） |
| [coverage_graph_bellman_ford_all_pairs_yuki.cpp](../current/submissions/coverage_graph_bellman_ford_all_pairs_yuki.cpp) | yukicoder: [Typical Shortest Path Sum (BellmanFord)](https://yukicoder.me/problems/no/1344) | 各始点の負辺を含む距離・多重辺・未到達infを除外した距離総和 |
| [coverage_graph_bellman_ford_constraints_yuki.cpp](../current/submissions/coverage_graph_bellman_ford_constraints_yuki.cpp) | yukicoder: [Plane Tickets (negative-cycle feasibility)](https://yukicoder.me/problems/no/1122) | 負辺を含む差分制約・負閉路の有無が実現可能性の出力へ反映・最大10^15の券枚数 |
| [coverage_graph_capacity_shirobako.cpp](../current/submissions/coverage_graph_capacity_shirobako.cpp) | yukicoder: [制作進行の宮森あおいです！（一般容量最大流）](https://yukicoder.me/problems/no/177) | mf_graph<int64_t>::add_edgeの容量0..10000・flow(s,t,flow_limit)の目標流量への到達判定・二部輸送網の容量ボトルネック/NG担当者 |
| [coverage_graph_floyd_warshall_all_pairs_yuki.cpp](../current/submissions/coverage_graph_floyd_warshall_all_pairs_yuki.cpp) | yukicoder: [Typical Shortest Path Sum (FloydWarshall)](https://yukicoder.me/problems/no/1344) | 負辺を含む有向全点対距離・多重辺の最小化・未到達infを除外した各行総和 |
| [coverage_graph_node_cycles_yuki.cpp](../current/submissions/coverage_graph_node_cycles_yuki.cpp) | yukicoder: [Teleportation Cycles (Node)](https://yukicoder.me/problems/no/3650) | make_node/Node::nの共有接続・break_cycleの真偽が全成分の閉路数へ直接反映・自己ループと複数成分 |
| [coverage_graph_spfa_all_pairs_yuki.cpp](../current/submissions/coverage_graph_spfa_all_pairs_yuki.cpp) | yukicoder: [Typical Shortest Path Sum (SPFA)](https://yukicoder.me/problems/no/1344) | 各始点の負辺を含む距離・多重辺・未到達infを除外した距離総和 |
| [coverage_graph_spfa_constraints_yuki.cpp](../current/submissions/coverage_graph_spfa_constraints_yuki.cpp) | yukicoder: [Plane Tickets (negative-cycle feasibility)](https://yukicoder.me/problems/no/1122) | 負辺を含む差分制約・負閉路の有無が実現可能性の出力へ反映・最大10^15の券枚数 |
| [coverage_hash_set_string_search.cpp](../current/submissions/coverage_hash_set_string_search.cpp) | yukicoder: [文字列検索（HashSet<string>）](https://yukicoder.me/problems/no/430) | 文字列insert/contains、実入力の各部分文字列の所属判定 |
| [coverage_rolling_hash_short_search.cpp](../current/submissions/coverage_rolling_hash_short_search.cpp) | yukicoder: [文字列検索（底27の短区間RollingHash）](https://yukicoder.me/problems/no/430) | RollingHash(s,27)・get(l,r)とwhole()の異なる個体間での比較・長い本文での短区間抽出 |
| [coverage_structure_line2d_triangle.cpp](../current/submissions/coverage_structure_line2d_triangle.cpp) | yukicoder: [Largest Triangle (LineContainer2D)](https://yukicoder.me/problems/no/2012) | 両係数が変わるmax_rounded_ll/min_rounded_ll・符号の異なる方向と点の追加 |

既存の重み付きrollback補完：[Yuki2293](../current/submissions/rollback_weighted_union_find_reset_constraints.cpp)・[AWC0100 M](../current/submissions/rollback_weighted_union_find_versioned_potential.cpp)。これらもオンライン未提出です。

## 前回AtCoderを維持した19本

18本に優先順位の高い問題か補足検証を追加しました。辞書順最小Topoは今回もABC223 Dを維持します。向き判定2本は優先順位の高いyukicoderへ移しますが、ほかのAtCoder問題は座標の広い範囲や非凸形状、異なる出力・APIを検証するため残します。

| 従来ソース | 追加する問題・扱い |
| --- | --- |
| [bellman_ford.cpp](../current/submissions/bellman_ford.cpp) | [Typical Shortest Path Sum (BellmanFord)](../current/submissions/coverage_graph_bellman_ford_all_pairs_yuki.cpp)・[Plane Tickets (negative-cycle feasibility)](../current/submissions/coverage_graph_bellman_ford_constraints_yuki.cpp)。併用 |
| [floyd_warshall.cpp](../current/submissions/floyd_warshall.cpp) | [Typical Shortest Path Sum (FloydWarshall)](../current/submissions/coverage_graph_floyd_warshall_all_pairs_yuki.cpp)。併用 |
| [geometry_circle_relation_full.cpp](../current/submissions/geometry_circle_relation_full.cpp) | [1円玉投げ](../current/submissions/coverage_geometry_coin_overlap_full.cpp)。併用 |
| [geometry_circle_relation_split.cpp](../current/submissions/geometry_circle_relation_split.cpp) | [1円玉投げ](../current/submissions/coverage_geometry_coin_overlap_split.cpp)。併用 |
| [geometry_contains_atcoder_full.cpp](../current/submissions/geometry_contains_atcoder_full.cpp) | [UFOを捕まえろ](../current/submissions/coverage_geometry_circle_in_diamond_full.cpp)。併用 |
| [geometry_contains_atcoder_split.cpp](../current/submissions/geometry_contains_atcoder_split.cpp) | [UFOを捕まえろ](../current/submissions/coverage_geometry_circle_in_diamond_split.cpp)。併用 |
| [geometry_orientation_atcoder_full.cpp](../current/submissions/geometry_orientation_atcoder_full.cpp) | [貫け！（向き判定）](../current/submissions/coverage_geometry_orientation_stabbing_full.cpp)。移行 |
| [geometry_orientation_atcoder_split.cpp](../current/submissions/geometry_orientation_atcoder_split.cpp) | [貫け！（向き判定）](../current/submissions/coverage_geometry_orientation_stabbing_split.cpp)。移行 |
| [geometry_point_segment_distance_full.cpp](../current/submissions/geometry_point_segment_distance_full.cpp) | [UFOを捕まえろ](../current/submissions/coverage_geometry_circle_in_diamond_full.cpp)。併用 |
| [geometry_point_segment_distance_split.cpp](../current/submissions/geometry_point_segment_distance_split.cpp) | [UFOを捕まえろ](../current/submissions/coverage_geometry_circle_in_diamond_split.cpp)。併用 |
| [geometry_segment_intersection_atcoder_full.cpp](../current/submissions/geometry_segment_intersection_atcoder_full.cpp) | [貫け！（線分交差）](../current/submissions/coverage_geometry_segment_stabbing_full.cpp)。併用 |
| [geometry_segment_intersection_atcoder_split.cpp](../current/submissions/geometry_segment_intersection_atcoder_split.cpp) | [貫け！（線分交差）](../current/submissions/coverage_geometry_segment_stabbing_split.cpp)。併用 |
| [geometry_triangle_area_full.cpp](../current/submissions/geometry_triangle_area_full.cpp) | [Greedy MtSaka](../current/submissions/coverage_geometry_convex_polygon_area_full.cpp)・[面積の求め方（多角形近似）](../current/submissions/coverage_geometry_parabola_polygon_area_full.cpp)。併用 |
| [geometry_triangle_area_split.cpp](../current/submissions/geometry_triangle_area_split.cpp) | [Greedy MtSaka](../current/submissions/coverage_geometry_convex_polygon_area_split.cpp)・[面積の求め方（多角形近似）](../current/submissions/coverage_geometry_parabola_polygon_area_split.cpp)。併用 |
| [hash_set_strings.cpp](../current/submissions/hash_set_strings.cpp) | [文字列検索（HashSet<string>）](../current/submissions/coverage_hash_set_string_search.cpp)。併用 |
| [next_combination.cpp](../current/submissions/next_combination.cpp) | [Maximum Independent Set（組合せ列挙）](../current/submissions/coverage_next_combination_independent_set.cpp)。併用 |
| [node_reachable.cpp](../current/submissions/node_reachable.cpp) | [Teleportation Cycles (Node)](../current/submissions/coverage_graph_node_cycles_yuki.cpp)。併用 |
| [spfa.cpp](../current/submissions/spfa.cpp) | [Typical Shortest Path Sum (SPFA)](../current/submissions/coverage_graph_spfa_all_pairs_yuki.cpp)・[Plane Tickets (negative-cycle feasibility)](../current/submissions/coverage_graph_spfa_constraints_yuki.cpp)。併用 |
| [topological_lexicographical.cpp](../current/submissions/topological_lexicographical.cpp) | 現AtCoderを維持。維持 |

各問題の制約と範囲は[manifest](../current/manifest.json)のcoverage/limitations、選定理由と未検証項目は[監査記録](../../Audit/verify-coverage-final-2026-11-15.json)を参照してください。
