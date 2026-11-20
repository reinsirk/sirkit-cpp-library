# 提出29本の結果とFastest比較（2026-11-16）

29本すべてAC。公式の提出本文と現行生成コードを、生成冒頭の注記だけを除外して照合した。
本体・main・I/O・最適化設定は変更していない。Submission URLは生成元にも保存し、判定とREADMEを更新した。コミットなし。

## Library Checker（28本）

時間はms。Fastestは公式ページと同じAC・時間昇順・言語指定なしの取得結果。
14問題すべて全言語最速とcpp最速が一致し、取得した最速提出はすべてis_latest=true。
ただし過去提出の実コンパイラ版・実行環境の完全一致は公開情報から証明できない。倍率は今回の単発提出時間同士の比較。

| ソース | 提出 | 時間 | Fastest | 差 | 倍率 |
|---|---|---:|---:|---:|---:|
| [preferred_fraction_rational_approximation.cpp](../current/submissions/preferred_fraction_rational_approximation.cpp) | [417350](https://judge.yosupo.jp/submission/417350) AC | 287 | [31](https://judge.yosupo.jp/submission/412924) | +256 | 9.26× |
| [preferred_max_flow_bipartite.cpp](../current/submissions/preferred_max_flow_bipartite.cpp) | [417351](https://judge.yosupo.jp/submission/417351) AC | 2112 | [70](https://judge.yosupo.jp/submission/396318) | +2042 | 30.17× |
| [coverage_next_combination_independent_set.cpp](../current/submissions/coverage_next_combination_independent_set.cpp) | [417352](https://judge.yosupo.jp/submission/417352) AC | 18 | [1](https://judge.yosupo.jp/submission/404912) | +17 | 18.00× |
| [coverage_mo_inversions.cpp](../current/submissions/coverage_mo_inversions.cpp) | [417353](https://judge.yosupo.jp/submission/417353) AC | 570 | [39](https://judge.yosupo.jp/submission/415486) | +531 | 14.62× |
| [coverage_subset_transform_full.cpp](../current/submissions/coverage_subset_transform_full.cpp) | [417354](https://judge.yosupo.jp/submission/417354) AC | 44 | [11](https://judge.yosupo.jp/submission/409492) | +33 | 4.00× |
| [coverage_subset_transform_split.cpp](../current/submissions/coverage_subset_transform_split.cpp) | [417355](https://judge.yosupo.jp/submission/417355) AC | 44 | [11](https://judge.yosupo.jp/submission/409492) | +33 | 4.00× |
| [coverage_sparse_gcd_holes_full.cpp](../current/submissions/coverage_sparse_gcd_holes_full.cpp) | [417357](https://judge.yosupo.jp/submission/417357) AC | 150 | [13](https://judge.yosupo.jp/submission/409724) | +137 | 11.54× |
| [coverage_sparse_gcd_holes_split.cpp](../current/submissions/coverage_sparse_gcd_holes_split.cpp) | [417358](https://judge.yosupo.jp/submission/417358) AC | 158 | [13](https://judge.yosupo.jp/submission/409724) | +145 | 12.15× |
| [coverage_structure_binary_ordered.cpp](../current/submissions/coverage_structure_binary_ordered.cpp) | [417359](https://judge.yosupo.jp/submission/417359) AC | 380 | [35](https://judge.yosupo.jp/submission/409360) | +345 | 10.86× |
| [coverage_structure_binary_xor_ids.cpp](../current/submissions/coverage_structure_binary_xor_ids.cpp) | [417360](https://judge.yosupo.jp/submission/417360) AC | 203 | [21](https://judge.yosupo.jp/submission/403013) | +182 | 9.67× |
| [coverage_structure_compressed_ordered.cpp](../current/submissions/coverage_structure_compressed_ordered.cpp) | [417361](https://judge.yosupo.jp/submission/417361) AC | 297 | [35](https://judge.yosupo.jp/submission/409360) | +262 | 8.49× |
| [coverage_structure_compressed_xor_ids.cpp](../current/submissions/coverage_structure_compressed_xor_ids.cpp) | [417362](https://judge.yosupo.jp/submission/417362) AC | 215 | [21](https://judge.yosupo.jp/submission/403013) | +194 | 10.24× |
| [coverage_structure_persistent_queue.cpp](../current/submissions/coverage_structure_persistent_queue.cpp) | [417363](https://judge.yosupo.jp/submission/417363) AC | 254 | [24](https://judge.yosupo.jp/submission/404575) | +230 | 10.58× |
| [coverage_structure_disjoint_composite.cpp](../current/submissions/coverage_structure_disjoint_composite.cpp) | [417364](https://judge.yosupo.jp/submission/417364) AC | 75 | [10](https://judge.yosupo.jp/submission/403012) | +65 | 7.50× |
| [coverage_structure_fenwick_predecessor.cpp](../current/submissions/coverage_structure_fenwick_predecessor.cpp) | [417365](https://judge.yosupo.jp/submission/417365) AC | 286 | [16](https://judge.yosupo.jp/submission/403023) | +270 | 17.88× |
| [coverage_structure_segment_predecessor.cpp](../current/submissions/coverage_structure_segment_predecessor.cpp) | [417366](https://judge.yosupo.jp/submission/417366) AC | 194 | [16](https://judge.yosupo.jp/submission/403023) | +178 | 12.12× |
| [preferred_data_heap_double.cpp](../current/submissions/preferred_data_heap_double.cpp) | [417367](https://judge.yosupo.jp/submission/417367) AC | 74 | [26](https://judge.yosupo.jp/submission/410602) | +48 | 2.85× |
| [preferred_data_radix_frequency_int64.cpp](../current/submissions/preferred_data_radix_frequency_int64.cpp) | [417368](https://judge.yosupo.jp/submission/417368) AC | 172 | [46](https://judge.yosupo.jp/submission/362490) | +126 | 3.74× |
| [preferred_data_radix_frequency_uint32.cpp](../current/submissions/preferred_data_radix_frequency_uint32.cpp) | [417369](https://judge.yosupo.jp/submission/417369) AC | 177 | [46](https://judge.yosupo.jp/submission/362490) | +131 | 3.85× |
| [zeta_and_full.cpp](../current/submissions/zeta_and_full.cpp) | [417370](https://judge.yosupo.jp/submission/417370) AC | 51 | [11](https://judge.yosupo.jp/submission/409492) | +40 | 4.64× |
| [zeta_or_as_and_full.cpp](../current/submissions/zeta_or_as_and_full.cpp) | [417371](https://judge.yosupo.jp/submission/417371) AC | 50 | [11](https://judge.yosupo.jp/submission/409492) | +39 | 4.55× |
| [zeta_set_as_and_full.cpp](../current/submissions/zeta_set_as_and_full.cpp) | [417372](https://judge.yosupo.jp/submission/417372) AC | 58 | [11](https://judge.yosupo.jp/submission/409492) | +47 | 5.27× |
| [zeta_gcd_conv_full.cpp](../current/submissions/zeta_gcd_conv_full.cpp) | [417373](https://judge.yosupo.jp/submission/417373) AC | 49 | [13](https://judge.yosupo.jp/submission/409724) | +36 | 3.77× |
| [zeta_gcd_transform_full.cpp](../current/submissions/zeta_gcd_transform_full.cpp) | [417374](https://judge.yosupo.jp/submission/417374) AC | 52 | [13](https://judge.yosupo.jp/submission/409724) | +39 | 4.00× |
| [zeta_lcm_conv_full.cpp](../current/submissions/zeta_lcm_conv_full.cpp) | [417375](https://judge.yosupo.jp/submission/417375) AC | 55 | [14](https://judge.yosupo.jp/submission/409729) | +41 | 3.93× |
| [zeta_lcm_transform_full.cpp](../current/submissions/zeta_lcm_transform_full.cpp) | [417376](https://judge.yosupo.jp/submission/417376) AC | 61 | [14](https://judge.yosupo.jp/submission/409729) | +47 | 4.36× |
| [zeta_sparse_gcd_full.cpp](../current/submissions/zeta_sparse_gcd_full.cpp) | [417377](https://judge.yosupo.jp/submission/417377) AC | 64 | [13](https://judge.yosupo.jp/submission/409724) | +51 | 4.92× |
| [zeta_sparse_gcd_split.cpp](../current/submissions/zeta_sparse_gcd_split.cpp) | [417378](https://judge.yosupo.jp/submission/417378) AC | 57 | [13](https://judge.yosupo.jp/submission/409724) | +44 | 4.38× |

## 差の読み方

- 最大流：2112対70ms（+2042ms）。最速は二部マッチング専用のCSR隣接・再割当処理、Verifyは汎用mf_graphの残余ネットワーク。専用化・表現・定数倍の違いがある。単位二部網のDinicも同じO(E√V)境界を持つので、30倍差を理論計算量だけの差とはしない。
- Moの転倒数：570対39ms。最速はMoではなくオフラインsweepと二段平方分割。APIの検証を保ったまま最速コードへ置き換えることはできない。
- 前後検索：Fenwick286 / Segment Tree194対16ms。最速は階層bit mapとbit探索を使い、比較するデータ構造自体が異なる。
- 分数：287対31ms。同系統のStern–Brocot移動だが、最速は整数剰余だけで更新し、VerifyはFractionの四則・gcd/約分を実行する。Fraction APIを外せばVerifyの目的を失う。
- 穴のある疎変換：150/158対13ms。Verifyは添字を2倍へ写し、最大キー2N・約1.5Nキーの約数閉包で疎経路を検証する。最速の通常の密GCD畳み込みとは計算内容・入力表現が異なる。
- 通常のAND/GCD/LCM畳み込みにも約3.8〜5.3倍の差が残る。同系統の変換で、法演算の表現・SIMD・入出力など実装面の再検討余地がある。最速解法の読取りによる分析で、各要因の寄与はまだ同条件で分離測定していない。

確認した主な問題の最速ソースの分析・行番号・留保は[分析監査](../../Audit/submissions29-fastest-analysis-2026-11-16.json)を参照。
ヒープ・最大独立集合・Ordered Set・XOR最小・頻度照会は時間比較まで確認し、最速ソースの詳細な原因分析は今回の対象から外した。
今回行ったのは提出結果の照合と比較で、新たな最適化・測定・Fable呼出しは行っていない。

## yukicoder

- [preferred_sieve_factorize_full.cpp](../current/submissions/preferred_sieve_factorize_full.cpp)：[1199507](https://yukicoder.me/submissions/1199507) **AC・1ms**。C++23（GCC15.3.0）。提出本文と現行版の差はSubmission注記のみ。Fastest比較の対象外。

## 記録

[結果監査](../../Audit/submissions29-results-2026-11-16.json)に各ソースSHA、判定、最速との比較、公式取得情報を保存した。
最適化済みの旧canonicalも、提出本文との注記限定一致を再確認してACの別表現へ結び付けた。別コードの旧ACを流用していない。
