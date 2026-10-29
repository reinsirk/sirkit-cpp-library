# 優先再提出の結果（2026-10-30）

優先17本はすべてAC。追加の整数版Prime Modを含む18本を確認した。Library Checkerの11本は提出本文と現行版の一致を確認。AtCoderの7本は画像でAC・時間を確認し、提出本文はHTTP403で未取得。

本体・main・共通入出力は今回変更していない。前回とのオンライン時間は別実行の比較であり、CPU環境やコンパイラの差を分離した本体測定ではない。

| 提出コード | 前回→今回 | 判定 | 本文照合 |
|---|---|---|
| [dynamic_fenwick_range.cpp](../Verify/current/submissions/dynamic_fenwick_range.cpp) | 58→45 ms | [AC](https://judge.yosupo.jp/submission/414007) | 一致 |
| [dynamic_fenwick_rectangle.cpp](../Verify/current/submissions/dynamic_fenwick_rectangle.cpp) | 433→414 ms | [AC](https://judge.yosupo.jp/submission/414008) | 一致 |
| [heap_priority_queue.cpp](../Verify/current/submissions/heap_priority_queue.cpp) | 32→36 ms | [AC](https://atcoder.jp/contests/abc234/submissions/80467513) | 画像確認・本文未取得 |
| [implicit_treap_affine.cpp](../Verify/current/submissions/implicit_treap_affine.cpp) | 1340→1292 ms | [AC](https://judge.yosupo.jp/submission/414009) | 一致 |
| [hash_set_membership.cpp](../Verify/current/submissions/hash_set_membership.cpp) | 50→44 ms | [AC](https://atcoder.jp/contests/abc344/submissions/80467519) | 画像確認・本文未取得 |
| [hash_set_toggle.cpp](../Verify/current/submissions/hash_set_toggle.cpp) | 9→9 ms | [AC](https://atcoder.jp/contests/abc073/submissions/80467524) | 画像確認・本文未取得 |
| [bellman_ford.cpp](../Verify/current/submissions/bellman_ford.cpp) | 10→3 ms | [AC](https://atcoder.jp/contests/abc137/submissions/80467528) | 画像確認・本文未取得 |
| [binomial_integer_split.cpp](../Verify/current/submissions/binomial_integer_split.cpp) | 41→41 ms | [AC](https://judge.yosupo.jp/submission/414010) | 一致 |
| [binomial_modint_split.cpp](../Verify/current/submissions/binomial_modint_split.cpp) | 41→41 ms | [AC](https://judge.yosupo.jp/submission/414017) | 一致 |
| [primality.cpp](../Verify/current/submissions/primality.cpp) | 88→88 ms | [AC](https://judge.yosupo.jp/submission/414012) | 一致 |
| [nearest_two_points.cpp](../Verify/current/submissions/nearest_two_points.cpp) | 89→78 ms | [AC](https://judge.yosupo.jp/submission/414013) | 一致 |
| [angle_sort.cpp](../Verify/current/submissions/angle_sort.cpp) | 38→31 ms | [AC](https://judge.yosupo.jp/submission/414014) | 一致 |
| [project_selection.cpp](../Verify/current/submissions/project_selection.cpp) | 1→1 ms | [AC](https://atcoder.jp/contests/typical90/submissions/80467555) | 画像確認・本文未取得 |
| [rollback_weighted_union_find.cpp](../Verify/current/submissions/rollback_weighted_union_find.cpp) | 15→15 ms | [AC](https://judge.yosupo.jp/submission/414015) | 一致 |
| [lis.cpp](../Verify/current/submissions/lis.cpp) | 43→36 ms | [AC](https://judge.yosupo.jp/submission/414016) | 一致 |
| [topological.cpp](../Verify/current/submissions/topological.cpp) | 22→22 ms | [AC](https://atcoder.jp/contests/dp/submissions/80467560) | 画像確認・本文未取得 |
| [matrix_rotate.cpp](../Verify/current/submissions/matrix_rotate.cpp) | 1→1 ms | [AC](https://atcoder.jp/contests/abc298/submissions/80467567) | 画像確認・本文未取得 |
| [binomial_integer_prime_mod_split.cpp](../Verify/current/submissions/binomial_integer_prime_mod_split.cpp) | 239→224 ms | [AC](https://judge.yosupo.jp/submission/414011) | 一致 |

## 遅くなった結果の扱い

heapは32→36ms。提出本文が未取得のため、オンライン結果のみで本体の退行と断定はしない。次に調べる対象として残す。

全体の最大時間が短くても、ケース別には次の増加がある。前回10ms以上、増加2ms以上かつ10%以上の全件を掲載した。この一覧は閾値未満の増加がないという意味ではない。

| 対象 | ケース | 前回→今回 |
|---|---|---|
| implicit_treap_affine | wrong_splay_killer_02 | 108→119 ms |
| binomial_integer_split | m_n_524288_00 | 23→29 ms |
| binomial_integer_split | m_prime_n_max_04 | 22→31 ms |
| binomial_modint_split | m_n_999983_00 | 15→22 ms |
| binomial_modint_split | m_prime_n_max_00 | 17→28 ms |
| binomial_modint_split | n_small_02 | 15→23 ms |
| nearest_two_points | small_random_07 | 36→42 ms |
| nearest_two_points | max_colinear_01 | 24→35 ms |
| lis | random_01 | 31→36 ms |
| binomial_integer_prime_mod_split | mod2_00 | 19→28 ms |

## 残る提出

[6時間作業時の一覧](resubmissions-six-hours-2026-10-29.md)は履歴として保持する。今回で18本の再提出待ちを解除し、残りは36本。優先17本の未提出はない。AtCoder7本の本文未照合は別に残し、再提出が必要という意味にはしていない。

| 提出コード | 問題 |
|---|---|
| [dynamic_fenwick_2d_rectangle.cpp](../Verify/current/submissions/dynamic_fenwick_2d_rectangle.cpp) | [提出先](https://judge.yosupo.jp/problem/point_add_rectangle_sum) |
| [dynamic_lichao_segment.cpp](../Verify/current/submissions/dynamic_lichao_segment.cpp) | [提出先](https://judge.yosupo.jp/problem/segment_add_get_min) |
| [dynamic_lichao_line.cpp](../Verify/current/submissions/dynamic_lichao_line.cpp) | [提出先](https://judge.yosupo.jp/problem/line_add_get_min) |
| [dynamic_union_find.cpp](../Verify/current/submissions/dynamic_union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/unionfind) |
| [hash_map.cpp](../Verify/current/submissions/hash_map.cpp) | [提出先](https://judge.yosupo.jp/problem/associative_array) |
| [line_container_min.cpp](../Verify/current/submissions/line_container_min.cpp) | [提出先](https://judge.yosupo.jp/problem/line_add_get_min) |
| [implicit_treap_deque.cpp](../Verify/current/submissions/implicit_treap_deque.cpp) | [提出先](https://judge.yosupo.jp/problem/deque_operate_all_composite) |
| [line_container_2d_min.cpp](../Verify/current/submissions/line_container_2d_min.cpp) | [提出先](https://judge.yosupo.jp/problem/line_add_get_min) |
| [hash_set_strings.cpp](../Verify/current/submissions/hash_set_strings.cpp) | [提出先](https://atcoder.jp/contests/abc164/tasks/abc164_c) |
| [dijkstra.cpp](../Verify/current/submissions/dijkstra.cpp) | [提出先](https://judge.yosupo.jp/problem/shortest_path) |
| [compressed_binary_trie.cpp](../Verify/current/submissions/compressed_binary_trie.cpp) | [提出先](https://judge.yosupo.jp/problem/set_xor_min) |
| [binomial_prime_split.cpp](../Verify/current/submissions/binomial_prime_split.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) |
| [binomial_modint_prime_mod_split.cpp](../Verify/current/submissions/binomial_modint_prime_mod_split.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) |
| [zeta_and_full.cpp](../Verify/current/submissions/zeta_and_full.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) |
| [zeta_and_split.cpp](../Verify/current/submissions/zeta_and_split.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) |
| [zeta_or_as_and_full.cpp](../Verify/current/submissions/zeta_or_as_and_full.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) |
| [zeta_or_as_and_split.cpp](../Verify/current/submissions/zeta_or_as_and_split.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) |
| [zeta_gcd_conv_full.cpp](../Verify/current/submissions/zeta_gcd_conv_full.cpp) | [提出先](https://judge.yosupo.jp/problem/gcd_convolution) |
| [zeta_gcd_conv_split.cpp](../Verify/current/submissions/zeta_gcd_conv_split.cpp) | [提出先](https://judge.yosupo.jp/problem/gcd_convolution) |
| [zeta_lcm_conv_full.cpp](../Verify/current/submissions/zeta_lcm_conv_full.cpp) | [提出先](https://judge.yosupo.jp/problem/lcm_convolution) |
| [zeta_lcm_conv_split.cpp](../Verify/current/submissions/zeta_lcm_conv_split.cpp) | [提出先](https://judge.yosupo.jp/problem/lcm_convolution) |
| [zeta_sieve_factorize_full.cpp](../Verify/current/submissions/zeta_sieve_factorize_full.cpp) | [提出先](https://atcoder.jp/contests/abc169/tasks/abc169_d) |
| [angle_sort_std.cpp](../Verify/current/submissions/angle_sort_std.cpp) | [提出先](https://judge.yosupo.jp/problem/sort_points_by_argument) |
| [zeta_sparse_gcd_full.cpp](../Verify/current/submissions/zeta_sparse_gcd_full.cpp) | [提出先](https://judge.yosupo.jp/problem/gcd_convolution) |
| [zeta_sparse_gcd_split.cpp](../Verify/current/submissions/zeta_sparse_gcd_split.cpp) | [提出先](https://judge.yosupo.jp/problem/gcd_convolution) |
| [geometry_convex_hull_full.cpp](../Verify/current/submissions/geometry_convex_hull_full.cpp) | [提出先](https://judge.yosupo.jp/problem/static_convex_hull) |
| [geometry_convex_hull_split.cpp](../Verify/current/submissions/geometry_convex_hull_split.cpp) | [提出先](https://judge.yosupo.jp/problem/static_convex_hull) |
| [union_find.cpp](../Verify/current/submissions/union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/unionfind) |
| [weighted_union_find.cpp](../Verify/current/submissions/weighted_union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/unionfind_with_potential) |
| [rollback_weighted_union_find_rollback.cpp](../Verify/current/submissions/rollback_weighted_union_find_rollback.cpp) | [提出先](https://judge.yosupo.jp/problem/persistent_unionfind) |
| [persistent_union_find.cpp](../Verify/current/submissions/persistent_union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/persistent_unionfind) |
| [rollback_union_find.cpp](../Verify/current/submissions/rollback_union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/persistent_unionfind) |
| [rolling_hash.cpp](../Verify/current/submissions/rolling_hash.cpp) | [提出先](https://judge.yosupo.jp/problem/zalgorithm) |
| [lca.cpp](../Verify/current/submissions/lca.cpp) | [提出先](https://judge.yosupo.jp/problem/lca) |
| [topological_lexicographical.cpp](../Verify/current/submissions/topological_lexicographical.cpp) | [提出先](https://atcoder.jp/contests/abc223/tasks/abc223_d) |
| [node_reachable.cpp](../Verify/current/submissions/node_reachable.cpp) | [提出先](https://atcoder.jp/contests/abc228/tasks/abc228_b) |

[詳細記録・全ケース比較](online-priority-resubmissions-2026-10-30.json) / [公開APIの取得内容](online-priority-resubmissions-2026-10-30-api.json)

各判定の過去の記録はjudgements.jsonのhistoryへ保持した。ソースコメントは最新の提出URLだけを表示する。
