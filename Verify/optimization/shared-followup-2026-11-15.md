# 共有部品変更が波及した既存10本の再確認（2026-11-15）

10本すべてをCodex→Fableの順で現行提出ソースSHAへ結び直した。今回の追加コード修正は不要と判断。
LC9本はmain・I/Oを含む全体、yukicoder1本は本体のみを再確認した。コミットは未実施。オンライン提出は29本の[結果照合](submissions29-results-2026-11-16.md)でAC・現行本文一致を確認済み。
旧ACや別SHAの旧レビューは、現行ソースの承認として引き継いでいない。

## 結果

- 変更前後を同一条件で比較。8本は実行コード等の記録した主要sectionが一致。
- 疎変換2本はキーが常に1..Nなので、今回追加した整列判定より前の連番高速経路で処理を終える。
- 代表的な大規模入力で変更前後を比較し、同一バイナリを使う対照測定で差を測定のばらつきと区別した。速度改善の追加成果とは扱わない。
- 現行SHAと一致する80構成・104実行の照合結果を再利用した。GCC/Clang、検査あり/なし、標準代替を含む。
- 変更分岐自体は別の独立疎API検証（16構成、4,944ケース、72例外検査）で補完した。

## 提出用ソース

| ソース | 確認範囲 | 状態 |
|---|---|---|
| [zeta_and_full.cpp](../current/submissions/zeta_and_full.cpp) | main・I/Oを含む全体 | AC・現行本文一致 |
| [zeta_or_as_and_full.cpp](../current/submissions/zeta_or_as_and_full.cpp) | main・I/Oを含む全体 | AC・現行本文一致 |
| [zeta_set_as_and_full.cpp](../current/submissions/zeta_set_as_and_full.cpp) | main・I/Oを含む全体 | AC・現行本文一致 |
| [zeta_gcd_conv_full.cpp](../current/submissions/zeta_gcd_conv_full.cpp) | main・I/Oを含む全体 | AC・現行本文一致 |
| [zeta_gcd_transform_full.cpp](../current/submissions/zeta_gcd_transform_full.cpp) | main・I/Oを含む全体 | AC・現行本文一致 |
| [zeta_lcm_conv_full.cpp](../current/submissions/zeta_lcm_conv_full.cpp) | main・I/Oを含む全体 | AC・現行本文一致 |
| [zeta_lcm_transform_full.cpp](../current/submissions/zeta_lcm_transform_full.cpp) | main・I/Oを含む全体 | AC・現行本文一致 |
| [zeta_sparse_gcd_full.cpp](../current/submissions/zeta_sparse_gcd_full.cpp) | main・I/Oを含む全体 | AC・現行本文一致 |
| [zeta_sparse_gcd_split.cpp](../current/submissions/zeta_sparse_gcd_split.cpp) | main・I/Oを含む全体 | AC・現行本文一致 |
| [preferred_sieve_factorize_full.cpp](../current/submissions/preferred_sieve_factorize_full.cpp) | ライブラリ本体 | AC・現行本文一致 |

[前回の追加19本](added-lc-optimization-2026-11-15.md)と合わせて29本すべてAC・現行本文一致を確認済み。
移行前の参考記録zeta_sieve_factorize_fullは提出対象から除外したまま。

検討・測定条件、正しさの再利用根拠、各SHA、Fableの結果は[最終監査](../../Audit/Performance/shared-followup-final-2026-11-15.json)を参照。
