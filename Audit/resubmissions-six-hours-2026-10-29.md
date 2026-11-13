# 6時間作業の再提出一覧（2026-10-29）

作業開始は17:40:48 JST、報告基準は23:40:48 JST。本体の最終版を対象に集計した。mainと共通入出力は全件、作業開始時とバイト単位で一致する。

再提出待ちは **54本**。下の17本は今回の変更した実行経路または正しさ修正の確認を優先する。残り37本は今回の対象外経路・同じ実行コード・以前からの未照合を含む。過去のACを現ソースのACへ書き換えていない。

記載した速度は同じmain・入出力・設定で測ったP-Coreのローカル値で、オンライン時間の予測ではない。

## 優先する17本

| 提出コード | 問題 | 確認点 |
|---|---|---|
| [dynamic_fenwick_range.cpp](../Verify/current/submissions/dynamic_fenwick_range.cpp) | [提出先](https://judge.yosupo.jp/problem/point_add_range_sum) | GCC既定の短区間。固定mainのpoint約9%、narrow約4%短縮。 |
| [dynamic_fenwick_rectangle.cpp](../Verify/current/submissions/dynamic_fenwick_rectangle.cpp) | [提出先](https://judge.yosupo.jp/problem/point_add_rectangle_sum) | 1D BITを外側Fenwickへ組み合わせる補助版。2D専用版を使う場合は低優先。 |
| [heap_priority_queue.cpp](../Verify/current/submissions/heap_priority_queue.cpp) | [提出先](https://atcoder.jp/contests/abc234/tasks/abc234_d) | GCC既定の混合2/4分岐。代表の固定main約28%短縮。 |
| [implicit_treap_affine.cpp](../Verify/current/submissions/implicit_treap_affine.cpp) | [提出先](https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum) | 反転・回転・範囲集計など。代表の固定main約2〜8%短縮。 |
| [hash_set_membership.cpp](../Verify/current/submissions/hash_set_membership.cpp) | [提出先](https://atcoder.jp/contests/abc344/tasks/abc344_c) | 整数挿入・所属照会。整数の実行コードを維持して借用キーの寿命不具合も修正。 |
| [hash_set_toggle.cpp](../Verify/current/submissions/hash_set_toggle.cpp) | [提出先](https://atcoder.jp/contests/abc073/tasks/abc073_c) | 整数の削除・再登録APIを高速化。借用キーの寿命修正後も整数mainは同じ実行コード。 |
| [bellman_ford.cpp](../Verify/current/submissions/bellman_ford.cpp) | [提出先](https://atcoder.jp/contests/abc137/tasks/abc137_e) | 負成分が連なる条件で固定main約76〜82%短縮。 |
| [binomial_integer_split.cpp](../Verify/current/submissions/binomial_integer_split.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient) | 法が2の冪の表構築を短縮。整数の固定mainで約5〜6%。 |
| [binomial_modint_split.cpp](../Verify/current/submissions/binomial_modint_split.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient) | 同じBinomialMod本体の表構築を利用。Mint main全体の短縮率は別扱い。 |
| [primality.cpp](../Verify/current/submissions/primality.cpp) | [提出先](https://judge.yosupo.jp/problem/primality_test) | GCCの64bit素数条件で約3%短縮。BMI2非対応・標準演算へ切替可。 |
| [nearest_two_points.cpp](../Verify/current/submissions/nearest_two_points.cpp) | [提出先](https://judge.yosupo.jp/problem/closest_pair) | 固定mainのGCC既定でrandom約6%、grid約9%、縦直線約70%短縮。 |
| [angle_sort.cpp](../Verify/current/submissions/angle_sort.cpp) | [提出先](https://judge.yosupo.jp/problem/sort_points_by_argument) | 固定mainの整列済み／少数乱れ約16〜19%、末尾移動約53〜56%短縮。 |
| [project_selection.cpp](../Verify/current/submissions/project_selection.cpp) | [提出先](https://atcoder.jp/contests/typical90/tasks/typical90_an) | 大規模で訪問頂点だけを初期化。確保失敗後の閾値跨ぎも修正。オンライン1ms問題は主に正しさ確認。 |
| [rollback_weighted_union_find.cpp](../Verify/current/submissions/rollback_weighted_union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/unionfind_with_potential) | 同じ親を持つ頂点の差。代表の星型API約15〜30%短縮。 |
| [lis.cpp](../Verify/current/submissions/lis.cpp) | [提出先](https://judge.yosupo.jp/problem/longest_increasing_subsequence) | 同じmainの大規模条件で約2〜19%短縮。 |
| [topological.cpp](../Verify/current/submissions/topological.cpp) | [提出先](https://atcoder.jp/contests/dp/tasks/dp_g) | 通常FIFOの初期候補登録。代表の疎グラフ約15〜16%短縮。 |
| [matrix_rotate.cpp](../Verify/current/submissions/matrix_rotate.cpp) | [提出先](https://atcoder.jp/contests/abc298/tasks/abc298_b) | 4/8byte要素の大規模回転を短縮。オンライン1ms問題は主に正しさ確認。 |

## ソース同期・以前からの未照合37本

同じ実行コードと確認できたものも、提出ソースの照合が完了していなければ記録上は再提出待ちとして残す。今回の速度改善だけを確認する場合は上の17本を先にする。

| 提出コード | 問題 | 区分 |
|---|---|---|
| [dynamic_fenwick_2d_rectangle.cpp](../Verify/current/submissions/dynamic_fenwick_2d_rectangle.cpp) | [提出先](https://judge.yosupo.jp/problem/point_add_rectangle_sum) | 1D共有部品の同期のみ。2D本体と4設定のmain実行コードは不変。 |
| [dynamic_lichao_segment.cpp](../Verify/current/submissions/dynamic_lichao_segment.cpp) | [提出先](https://judge.yosupo.jp/problem/segment_add_get_min) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [dynamic_lichao_line.cpp](../Verify/current/submissions/dynamic_lichao_line.cpp) | [提出先](https://judge.yosupo.jp/problem/line_add_get_min) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [dynamic_union_find.cpp](../Verify/current/submissions/dynamic_union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/unionfind) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [hash_map.cpp](../Verify/current/submissions/hash_map.cpp) | [提出先](https://judge.yosupo.jp/problem/associative_array) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [line_container_min.cpp](../Verify/current/submissions/line_container_min.cpp) | [提出先](https://judge.yosupo.jp/problem/line_add_get_min) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [implicit_treap_deque.cpp](../Verify/current/submissions/implicit_treap_deque.cpp) | [提出先](https://judge.yosupo.jp/problem/deque_operate_all_composite) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [line_container_2d_min.cpp](../Verify/current/submissions/line_container_2d_min.cpp) | [提出先](https://judge.yosupo.jp/problem/line_add_get_min) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [hash_set_strings.cpp](../Verify/current/submissions/hash_set_strings.cpp) | [提出先](https://atcoder.jp/contests/abc164/tasks/abc164_c) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [dijkstra.cpp](../Verify/current/submissions/dijkstra.cpp) | [提出先](https://judge.yosupo.jp/problem/shortest_path) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [compressed_binary_trie.cpp](../Verify/current/submissions/compressed_binary_trie.cpp) | [提出先](https://judge.yosupo.jp/problem/set_xor_min) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [binomial_integer_prime_mod_split.cpp](../Verify/current/submissions/binomial_integer_prime_mod_split.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [binomial_prime_split.cpp](../Verify/current/submissions/binomial_prime_split.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [binomial_modint_prime_mod_split.cpp](../Verify/current/submissions/binomial_modint_prime_mod_split.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [zeta_and_full.cpp](../Verify/current/submissions/zeta_and_full.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) | 今回の整数向け集合変換の変更経路を使わない。Mod型等の提出。 |
| [zeta_and_split.cpp](../Verify/current/submissions/zeta_and_split.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) | 今回の整数向け集合変換の変更経路を使わない。Mod型等の提出。 |
| [zeta_or_as_and_full.cpp](../Verify/current/submissions/zeta_or_as_and_full.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) | 今回の整数向け集合変換の変更経路を使わない。Mod型等の提出。 |
| [zeta_or_as_and_split.cpp](../Verify/current/submissions/zeta_or_as_and_split.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) | 今回の整数向け集合変換の変更経路を使わない。Mod型等の提出。 |
| [zeta_gcd_conv_full.cpp](../Verify/current/submissions/zeta_gcd_conv_full.cpp) | [提出先](https://judge.yosupo.jp/problem/gcd_convolution) | 今回の整数向け集合変換の変更経路を使わない。Mod型等の提出。 |
| [zeta_gcd_conv_split.cpp](../Verify/current/submissions/zeta_gcd_conv_split.cpp) | [提出先](https://judge.yosupo.jp/problem/gcd_convolution) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [zeta_lcm_conv_full.cpp](../Verify/current/submissions/zeta_lcm_conv_full.cpp) | [提出先](https://judge.yosupo.jp/problem/lcm_convolution) | 今回の整数向け集合変換の変更経路を使わない。Mod型等の提出。 |
| [zeta_lcm_conv_split.cpp](../Verify/current/submissions/zeta_lcm_conv_split.cpp) | [提出先](https://judge.yosupo.jp/problem/lcm_convolution) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [zeta_sieve_factorize_full.cpp](../Verify/current/submissions/zeta_sieve_factorize_full.cpp) | [提出先](https://atcoder.jp/contests/abc169/tasks/abc169_d) | 今回の整数向け集合変換の変更経路を使わない。Mod型等の提出。 |
| [angle_sort_std.cpp](../Verify/current/submissions/angle_sort_std.cpp) | [提出先](https://judge.yosupo.jp/problem/sort_points_by_argument) | std::sort比較器版。今回の前後で4設定の実行コードが同一。以前の未照合は維持。 |
| [zeta_sparse_gcd_full.cpp](../Verify/current/submissions/zeta_sparse_gcd_full.cpp) | [提出先](https://judge.yosupo.jp/problem/gcd_convolution) | 今回の整数向け集合変換の変更経路を使わない。Mod型等の提出。 |
| [zeta_sparse_gcd_split.cpp](../Verify/current/submissions/zeta_sparse_gcd_split.cpp) | [提出先](https://judge.yosupo.jp/problem/gcd_convolution) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [geometry_convex_hull_full.cpp](../Verify/current/submissions/geometry_convex_hull_full.cpp) | [提出先](https://judge.yosupo.jp/problem/static_convex_hull) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [geometry_convex_hull_split.cpp](../Verify/current/submissions/geometry_convex_hull_split.cpp) | [提出先](https://judge.yosupo.jp/problem/static_convex_hull) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [union_find.cpp](../Verify/current/submissions/union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/unionfind) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [weighted_union_find.cpp](../Verify/current/submissions/weighted_union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/unionfind_with_potential) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [rollback_weighted_union_find_rollback.cpp](../Verify/current/submissions/rollback_weighted_union_find_rollback.cpp) | [提出先](https://judge.yosupo.jp/problem/persistent_unionfind) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [persistent_union_find.cpp](../Verify/current/submissions/persistent_union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/persistent_unionfind) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [rollback_union_find.cpp](../Verify/current/submissions/rollback_union_find.cpp) | [提出先](https://judge.yosupo.jp/problem/persistent_unionfind) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [rolling_hash.cpp](../Verify/current/submissions/rolling_hash.cpp) | [提出先](https://judge.yosupo.jp/problem/zalgorithm) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [lca.cpp](../Verify/current/submissions/lca.cpp) | [提出先](https://judge.yosupo.jp/problem/lca) | 作業開始前からの再提出待ち。本作業で本体追加変更なし。 |
| [topological_lexicographical.cpp](../Verify/current/submissions/topological_lexicographical.cpp) | [提出先](https://atcoder.jp/contests/abc223/tasks/abc223_d) | 今回の本体変更を含むが、変更したAPIをmainが使わない等。 |
| [node_reachable.cpp](../Verify/current/submissions/node_reachable.cpp) | [提出先](https://atcoder.jp/contests/abc228/tasks/abc228_b) | nthを使わないmain。今回の前後で8設定の実行コードが同一。 |

## 未提出の補助版

以下は再提出ではなく、新規提出をする場合の候補。全機能版／分割版の重複も含む。

| 提出コード | 問題 |
|---|---|
| [binomial_integer.cpp](../Verify/current/submissions/binomial_integer.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient) |
| [binomial_modint.cpp](../Verify/current/submissions/binomial_modint.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient) |
| [binomial_integer_prime_mod.cpp](../Verify/current/submissions/binomial_integer_prime_mod.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) |
| [binomial_modint_prime_mod.cpp](../Verify/current/submissions/binomial_modint_prime_mod.cpp) | [提出先](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) |
| [zeta_set_as_and_full.cpp](../Verify/current/submissions/zeta_set_as_and_full.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) |
| [zeta_set_as_and_split.cpp](../Verify/current/submissions/zeta_set_as_and_split.cpp) | [提出先](https://judge.yosupo.jp/problem/bitwise_and_convolution) |
| [zeta_gcd_transform_full.cpp](../Verify/current/submissions/zeta_gcd_transform_full.cpp) | [提出先](https://judge.yosupo.jp/problem/gcd_convolution) |
| [zeta_lcm_transform_full.cpp](../Verify/current/submissions/zeta_lcm_transform_full.cpp) | [提出先](https://judge.yosupo.jp/problem/lcm_convolution) |
| [implicit_treap_erase_between.cpp](../Verify/current/submissions/implicit_treap_erase_between.cpp) | [提出先](https://atcoder.jp/contests/abc421/tasks/abc421_f) |

## 最新のFloyd–Warshall

[80459239](https://atcoder.jp/contests/abc012/submissions/80459239)は、画像とソースコメントからAC・7msを確認した。今回の6時間では本体を変更していない。提出ソース本文を取得できていないため、現ソースとの完全一致を確認したという意味ではない。

[最終Verify記録](verify-six-hours-final-2026-10-29.json)と各ライブラリのperformance/review監査に、採否・測定条件・不採用案・残差を保存している。
