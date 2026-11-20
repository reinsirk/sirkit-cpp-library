# Verify先の見直し（2026-11-15）

最新の再調査・追加39本と併用先は[検証範囲の一覧](verify-coverage-2026-11-15.md)を参照してください。以下は前回の提出先見直し時点の記録です。

優先順位は **Library Checker → yukicoder → AtCoder → AOJ** です。AtCoderの旧34本を見直し、検証したいAPIの結果が解答に反映される問題を選びました。
旧15本を13本の新規提出へ移し、19本は現在のAtCoderを維持します。同じライブラリへの重複した入口はまとめています。

## 新しく提出するコード

全件ローカル検証済み・オンライン未提出です。旧AtCoderのACは引き継ぎません。ライブラリ本体を含む下記ファイルを提出し、Submission URLを追記してください。

| ソース | 提出先・問題 | 移行元 |
| --- | --- | --- |
| [preferred_data_heap_double.cpp](../current/submissions/preferred_data_heap_double.cpp) | Library Checker: [Double-Ended Priority Queue (Heap)](https://judge.yosupo.jp/problem/double_ended_priority_queue) | `heap_priority_queue.cpp` |
| [preferred_data_radix_frequency_int64.cpp](../current/submissions/preferred_data_radix_frequency_int64.cpp) | Library Checker: [Static Range Frequency (RadixSort int64)](https://judge.yosupo.jp/problem/static_range_frequency) | `radix_sort.cpp` |
| [preferred_data_radix_frequency_uint32.cpp](../current/submissions/preferred_data_radix_frequency_uint32.cpp) | Library Checker: [Static Range Frequency (RadixSort uint32)](https://judge.yosupo.jp/problem/static_range_frequency) | `radix_sort_uint32.cpp` |
| [preferred_fraction_rational_approximation.cpp](../current/submissions/preferred_fraction_rational_approximation.cpp) | Library Checker: [Rational Approximation（Fractionの比較・四則）](https://judge.yosupo.jp/problem/rational_approximation) | `fraction_sort.cpp`, `fraction_arithmetic.cpp` |
| [preferred_max_flow_bipartite.cpp](../current/submissions/preferred_max_flow_bipartite.cpp) | Library Checker: [Matching on Bipartite Graph (mf_graph)](https://judge.yosupo.jp/problem/bipartitematching) | `max_flow_grid.cpp` |
| [preferred_data_hash_set_and_yuki.cpp](../current/submissions/preferred_data_hash_set_and_yuki.cpp) | yukicoder: [AND Set (HashSet)](https://yukicoder.me/problems/no/2195) | `hash_set_membership.cpp`, `hash_set_toggle.cpp` |
| [preferred_matrix_rotate_grid.cpp](../current/submissions/preferred_matrix_rotate_grid.cpp) | yukicoder: [回転拡大（grid）](https://yukicoder.me/problems/no/565) | `matrix_rotate_grid.cpp` |
| [preferred_matrix_rotate_integer.cpp](../current/submissions/preferred_matrix_rotate_integer.cpp) | yukicoder: [回転拡大（integer）](https://yukicoder.me/problems/no/565) | `matrix_rotate.cpp` |
| [preferred_project_selection_yuki.cpp](../current/submissions/preferred_project_selection_yuki.cpp) | yukicoder: [植林 (binary submodular optimization)](https://yukicoder.me/problems/no/957) | `project_selection.cpp` |
| [preferred_sieve_factorize_full.cpp](../current/submissions/preferred_sieve_factorize_full.cpp) | yukicoder: [素因数ゲーム（IntegerSieve）](https://yukicoder.me/problems/no/2) | `zeta_sieve_factorize_full.cpp` |
| [preferred_sieve_factorize_split.cpp](../current/submissions/preferred_sieve_factorize_split.cpp) | yukicoder: [素因数ゲーム（IntegerSieve）](https://yukicoder.me/problems/no/2) | `zeta_sieve_factorize_split.cpp` |
| [preferred_topological_yuki.cpp](../current/submissions/preferred_topological_yuki.cpp) | yukicoder: [Matrix Exponentiation (topological order)](https://yukicoder.me/problems/no/1660) | `topological.cpp` |
| [preferred_trie_dictionary.cpp](../current/submissions/preferred_trie_dictionary.cpp) | yukicoder: [文字列検索（Trie）](https://yukicoder.me/problems/no/430) | `trie_dictionary.cpp` |

## AtCoderを維持するコード

今回の調査では、上位の提出先に同じAPI範囲を自然に検証できる問題を確認できませんでした。上位問題の不在を網羅的に証明したものではありません。AOJの候補があっても、現在はAtCoderを優先します。

| ソース | 提出先 |
| --- | --- |
| [bellman_ford.cpp](../current/submissions/bellman_ford.cpp) | [E - Coins Respawn](https://atcoder.jp/contests/abc137/tasks/abc137_e) |
| [floyd_warshall.cpp](../current/submissions/floyd_warshall.cpp) | [D - バスと避けられない運命](https://atcoder.jp/contests/abc012/tasks/abc012_4) |
| [geometry_circle_relation_full.cpp](../current/submissions/geometry_circle_relation_full.cpp) | [035 - Two Circles](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_ag) |
| [geometry_circle_relation_split.cpp](../current/submissions/geometry_circle_relation_split.cpp) | [035 - Two Circles](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_ag) |
| [geometry_contains_atcoder_full.cpp](../current/submissions/geometry_contains_atcoder_full.cpp) | [098 - Polygon and Point](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_bu) |
| [geometry_contains_atcoder_split.cpp](../current/submissions/geometry_contains_atcoder_split.cpp) | [098 - Polygon and Point](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_bu) |
| [geometry_orientation_atcoder_full.cpp](../current/submissions/geometry_orientation_atcoder_full.cpp) | [C - Convex Quadrilateral](https://atcoder.jp/contests/abc266/tasks/abc266_c) |
| [geometry_orientation_atcoder_split.cpp](../current/submissions/geometry_orientation_atcoder_split.cpp) | [C - Convex Quadrilateral](https://atcoder.jp/contests/abc266/tasks/abc266_c) |
| [geometry_point_segment_distance_full.cpp](../current/submissions/geometry_point_segment_distance_full.cpp) | [033 - Distance](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_ae) |
| [geometry_point_segment_distance_split.cpp](../current/submissions/geometry_point_segment_distance_split.cpp) | [033 - Distance](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_ae) |
| [geometry_segment_intersection_atcoder_full.cpp](../current/submissions/geometry_segment_intersection_atcoder_full.cpp) | [037 - Intersection](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_ah) |
| [geometry_segment_intersection_atcoder_split.cpp](../current/submissions/geometry_segment_intersection_atcoder_split.cpp) | [037 - Intersection](https://atcoder.jp/contests/math-and-algorithm/tasks/math_and_algorithm_ah) |
| [geometry_triangle_area_full.cpp](../current/submissions/geometry_triangle_area_full.cpp) | [C - 直訴](https://atcoder.jp/contests/abc002/tasks/abc002_3) |
| [geometry_triangle_area_split.cpp](../current/submissions/geometry_triangle_area_split.cpp) | [C - 直訴](https://atcoder.jp/contests/abc002/tasks/abc002_3) |
| [hash_set_strings.cpp](../current/submissions/hash_set_strings.cpp) | [ABC164 C - gacha](https://atcoder.jp/contests/abc164/tasks/abc164_c) |
| [next_combination.cpp](../current/submissions/next_combination.cpp) | [E - Modulo MST](https://atcoder.jp/contests/abc328/tasks/abc328_e) |
| [node_reachable.cpp](../current/submissions/node_reachable.cpp) | [ABC228 B - Takahashi's Secret](https://atcoder.jp/contests/abc228/tasks/abc228_b) |
| [spfa.cpp](../current/submissions/spfa.cpp) | [E - Coins Respawn](https://atcoder.jp/contests/abc137/tasks/abc137_e) |
| [topological_lexicographical.cpp](../current/submissions/topological_lexicographical.cpp) | [D - Restricted Permutation](https://atcoder.jp/contests/abc223/tasks/abc223_d) |

問題ごとの検証範囲・不足は[manifest](../current/manifest.json)のcoverage/limitations、候補を採らない理由は[調査記録](../../Audit/verify-priority-final-2026-11-15.json)を参照してください。旧問題・判定は履歴とローカル回帰に残し、[提出方針](../current/submission_policy.json)で再提出対象から分離しています。
