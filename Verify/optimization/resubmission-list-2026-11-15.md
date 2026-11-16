# 再提出・照合待ち（2026-11-15）

最新の再調査・追加39本と併用先は[検証範囲の一覧](verify-coverage-2026-11-15.md)を参照してください。以下は前回の提出先見直し時点の記録です。

今回取得したLibrary Checkerの72提出はすべてACです。その後の改修を旧ACへ引き継がず、現在の全文で照合しています。最新の全件集計は[Verify状況](../current/status.md)を参照してください。

## 二項係数の6本は再提出済み

全件AC・提出本文一致を確認しました。確認時点のFastestは139msです。

| ソース | 提出 | 時間 | Fastestとの差 |
| --- | --- | ---: | ---: |
| binomial_integer_prime_mod.cpp | [AC](https://judge.yosupo.jp/submission/417291) | 150ms | +11ms |
| binomial_integer_prime_mod_split.cpp | [AC](https://judge.yosupo.jp/submission/417290) | 148ms | +9ms |
| binomial_modint_prime_mod.cpp | [AC](https://judge.yosupo.jp/submission/417293) | 164ms | +25ms |
| binomial_modint_prime_mod_split.cpp | [AC](https://judge.yosupo.jp/submission/417292) | 160ms | +21ms |
| binomial_prime.cpp | [AC](https://judge.yosupo.jp/submission/417295) | 140ms | +1ms |
| binomial_prime_split.cpp | [AC](https://judge.yosupo.jp/submission/417294) | 139ms | +0ms |

## 実装・mainが変わったLibrary Checkerの6本：再提出済み

6本すべてAC・提出本文と現在のコードの一致を確認しました。

| ソース | 提出 | 時間 |
| --- | --- | ---: |
| [dijkstra.cpp](../current/submissions/dijkstra.cpp) | [AC](https://judge.yosupo.jp/submission/417299) | 78ms |
| [dynamic_lichao_line.cpp](../current/submissions/dynamic_lichao_line.cpp) | [AC](https://judge.yosupo.jp/submission/417300) | 87ms |
| [dynamic_lichao_segment.cpp](../current/submissions/dynamic_lichao_segment.cpp) | [AC](https://judge.yosupo.jp/submission/417301) | 329ms |
| [implicit_treap_affine.cpp](../current/submissions/implicit_treap_affine.cpp) | [AC](https://judge.yosupo.jp/submission/417303) | 1135ms |
| [implicit_treap_deque.cpp](../current/submissions/implicit_treap_deque.cpp) | [AC](https://judge.yosupo.jp/submission/417302) | 107ms |
| [line_container_2d_min.cpp](../current/submissions/line_container_2d_min.cpp) | [AC](https://judge.yosupo.jp/submission/417304) | 267ms |

## コード先頭のAPI説明だけを変えた8本

**利用者指定により再提出不要です。** 当該ソースSHAだけを再提出待ちから除外し、新規のオンラインAC確認とは区別します。実装・main・入出力を変更した場合、この免除は引き継ぎません。

- [binomial_integer.cpp](../current/submissions/binomial_integer.cpp)
- [binomial_integer_split.cpp](../current/submissions/binomial_integer_split.cpp)
- [binomial_modint.cpp](../current/submissions/binomial_modint.cpp)
- [binomial_modint_split.cpp](../current/submissions/binomial_modint_split.cpp)
- [dynamic_fenwick_2d_rectangle.cpp](../current/submissions/dynamic_fenwick_2d_rectangle.cpp)
- [dynamic_fenwick_range.cpp](../current/submissions/dynamic_fenwick_range.cpp)
- [dynamic_fenwick_rectangle.cpp](../current/submissions/dynamic_fenwick_rectangle.cpp)
- [hash_map.cpp](../current/submissions/hash_map.cpp)

## URL注記だけの差を自動照合できない2本

次の2本は実装を変更していません。マクロの行継続があり、今回の厳格な冒頭正規化の対象外です。提出済みACの記録は保持し、現行ACへの自動昇格はしていません。

- [nearest_two_points.cpp](../current/submissions/nearest_two_points.cpp)
- [rolling_hash.cpp](../current/submissions/rolling_hash.cpp)

## 新しく用意した重み付きrollbackの検証

- [rollback_weighted_union_find_reset_constraints.cpp](../current/submissions/rollback_weighted_union_find_reset_constraints.cpp)：yukicoder No.2293。非零重みと全復元。
- [rollback_weighted_union_find_versioned_potential.cpp](../current/submissions/rollback_weighted_union_find_versioned_potential.cpp)：AWC0100 M。版の分岐・非零重み・差分照会を補足。

どちらもローカル検証済み・オンライン未提出です。既存AtCoder版の未照合分も全件集計へ残しています。

## AtCoder34本のVerify先の見直し

旧15本を新規13提出（LC5・yukicoder8）へ移し、19本はAtCoderを維持します。新規提出はオンライン未提出です。提出するファイル名と問題は[移行一覧](verify-targets-2026-11-15.md)を参照してください。

移行前のAtCoder提出は参考履歴に分け、再提出対象から除外しました。旧ACを新問題や改修後ソースへ引き継いではいません。
