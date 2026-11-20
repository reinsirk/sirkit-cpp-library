# 追加LC Verifyの最適化（2026-11-15）

19本すべてでCodexが検討・測定し、Fableが独立にレビューした後、提案を実測して採否を決めた。
最適化時の提出ソースSHAにstage記録を結び付けた。その後の[提出結果照合](submissions29-results-2026-11-16.md)で、19本がすべてACであり、提出本文が現行コードと一致することを確認した。コミットなし。

- 分数：Codexのギャップ更新で難入力を約42%、Fableの最終更新省略でその版からさらに約2〜10%短縮。
- 永続キュー：公開reserveを使い、計測profileで約340→146ms。
- Segment Treeの前後検索：Codex後の同設定からFable案で約30〜52%短縮、dense最大RSSは約179→55MiB。
- 疎なGCD変換：main・I/O・O2を固定した本体差分で約17〜23%短縮。末尾1組だけ逆転した配列では約1.6〜2.4%退行。
- Mo：ブロック幅変更で最大random約4%、二値約12%短縮。RadixSortのuint32索引もCodex版から主要3profileで約3〜5%短縮。

入力・コンパイラ・設定をそろえてローカルで測定した。全入力で一律に速くなることや、オンラインでFastestになることは保証しない。
Radix int64の小表等、残る退行と不採用案は[監査記録](../../Audit/added-lc-optimization-final-2026-11-15.json)から辿れる。

## 今回の19本

提出先はすべてLibrary Checker。リンク先は本体・main・I/Oを含む提出用1ファイル。

| ソース | 採用内容 |
|---|---|
| [coverage_next_combination_independent_set.cpp](../current/submissions/coverage_next_combination_independent_set.cpp) | 右半分DPを1byteの個数へ変更し、頂点集合は最後に復元。 |
| [coverage_mo_inversions.cpp](../current/submissions/coverage_mo_inversions.cpp) | 公開block引数をN*sqrt(2/max(1,Q))に調整。 |
| [coverage_subset_transform_full.cpp](../current/submissions/coverage_subset_transform_full.cpp) | driverの法加減算を整理。共有疎変換変更も反映。 |
| [coverage_subset_transform_split.cpp](../current/submissions/coverage_subset_transform_split.cpp) | driverの法加減算を整理。 |
| [coverage_sparse_gcd_holes_full.cpp](../current/submissions/coverage_sparse_gcd_holes_full.cpp) | 入力一時配列を除き、整列済みキーの再sortを省略。 |
| [coverage_sparse_gcd_holes_split.cpp](../current/submissions/coverage_sparse_gcd_holes_split.cpp) | 入力一時配列を除き、整列済みキーの再sortを省略。 |
| [coverage_structure_binary_ordered.cpp](../current/submissions/coverage_structure_binary_ordered.cpp) | 制約内の型幅とGCC設定を選択。 |
| [coverage_structure_binary_xor_ids.cpp](../current/submissions/coverage_structure_binary_xor_ids.cpp) | mainは維持しGCC設定を選択。 |
| [coverage_structure_compressed_ordered.cpp](../current/submissions/coverage_structure_compressed_ordered.cpp) | 公開reserveを使用し出力型を縮小。 |
| [coverage_structure_compressed_xor_ids.cpp](../current/submissions/coverage_structure_compressed_xor_ids.cpp) | mainは維持しGCC設定を選択。 |
| [coverage_structure_persistent_queue.cpp](../current/submissions/coverage_structure_persistent_queue.cpp) | 公開reserveで永続木の再確保を抑制。 |
| [coverage_structure_disjoint_composite.cpp](../current/submissions/coverage_structure_disjoint_composite.cpp) | 照会を32bitへ圧縮し、公開操作用配列を予約。 |
| [coverage_structure_fenwick_predecessor.cpp](../current/submissions/coverage_structure_fenwick_predecessor.cpp) | 疎な初期集合を直接登録し、同値更新を省略。 |
| [coverage_structure_segment_predecessor.cpp](../current/submissions/coverage_structure_segment_predecessor.cpp) | 疎な初期化・同値更新省略に加え、uint8のORモノイド。 |
| [preferred_data_heap_double.cpp](../current/submissions/preferred_data_heap_double.cpp) | 反対側Heapの先頭が同じなら即時削除。 |
| [preferred_data_radix_frequency_int64.cpp](../current/submissions/preferred_data_radix_frequency_int64.cpp) | RadixSort結果から値別の位置列・探索索引を構築。 |
| [preferred_data_radix_frequency_uint32.cpp](../current/submissions/preferred_data_radix_frequency_uint32.cpp) | 値別の位置列・探索索引を構築し、索引上限18bitを採用。 |
| [preferred_max_flow_bipartite.cpp](../current/submissions/preferred_max_flow_bipartite.cpp) | 辺が少ない場合の孤立点除去・公開reserve・流量上限。 |
| [preferred_fraction_rational_approximation.cpp](../current/submissions/preferred_fraction_rational_approximation.cpp) | Fractionのギャップを差分更新し、最終回の不要な更新を省略。 |

## 共有部品の変更が波及する既存オンラインVerify

下記11本にも疎変換の本体変更を反映し、各8構成の入力例を照合した。提出対象は10本、移行前の参考記録は1本。
今回の19本とは別枠。提出対象10本も[Codex→Fableの再確認](shared-followup-2026-11-15.md)を完了した。[提出結果照合](submissions29-results-2026-11-16.md)で当該10本のAC・現行本文一致も確認済み。旧ACを別コードへ移していない。
提出対象はLC9本とyukicoder1本。旧zeta_sieve_factorize_fullは移行前の参考記録のため再提出しない。ローカル専用4本への波及も照合済み。

- [zeta_and_full.cpp](../current/submissions/zeta_and_full.cpp)
- [zeta_or_as_and_full.cpp](../current/submissions/zeta_or_as_and_full.cpp)
- [zeta_set_as_and_full.cpp](../current/submissions/zeta_set_as_and_full.cpp)
- [zeta_gcd_conv_full.cpp](../current/submissions/zeta_gcd_conv_full.cpp)
- [zeta_gcd_transform_full.cpp](../current/submissions/zeta_gcd_transform_full.cpp)
- [zeta_lcm_conv_full.cpp](../current/submissions/zeta_lcm_conv_full.cpp)
- [zeta_lcm_transform_full.cpp](../current/submissions/zeta_lcm_transform_full.cpp)
- [zeta_sieve_factorize_full.cpp](../current/submissions/zeta_sieve_factorize_full.cpp)
- [zeta_sparse_gcd_full.cpp](../current/submissions/zeta_sparse_gcd_full.cpp)
- [zeta_sparse_gcd_split.cpp](../current/submissions/zeta_sparse_gcd_split.cpp)
- [preferred_sieve_factorize_full.cpp](../current/submissions/preferred_sieve_factorize_full.cpp)

## 検証と利用枠

採用版をGCC 15.2 / Clang 21、検査あり/なし、拡張利用/標準代替で照合。
共通本体の影響18本は144ビルド全成功。疎APIの独立参照4,944ケース・72例外検査も成功。
生成物の通常283本・派生1026本を同期。Python検査497件成功（意図した除外2件）。
Fableは承認済みアカウント・指定モデルで19件をレビュー。最終確認時は5時間58%、週間Fable56%、追加課金なし。
詳細・SHA・採否根拠・ローカル生記録ZIPへの参照は[最終監査](../../Audit/added-lc-optimization-final-2026-11-15.json)。
