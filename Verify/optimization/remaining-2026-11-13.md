# 残りのライブラリの最適化

当初101ソースのCodex検討は完了しています。旧Graph17ソースはFable検討と再提出済みで、提出時のLC10件は本文照合済みAC、AtCoder7件は画像AC・実提出全文未照合です。別枠のZ Algorithm（416725）も本文照合済みACです。

| 現在版の次の状態 | 件数 | 内訳 |
| --- | ---: | --- |
| `complete` | 8 | 現在版の提出本文照合済みACとFable完了 |
| `confirm_source` | 7 | AtCoder画像AC。実提出全文の確認待ちで再提出要求は出さない |
| `resubmit` | 87 | 現在版Fable検討済み。オンライン再提出が必要 |

元のFable待ち84件は、現在版の検討完了84件・未完了0件です。元84行のSHA・receiptはJSONの`original_pending_fable`に保全し、現在版は`pending_fable`と`completed_fable`へ分けています。

Dijkstra・LCAの辺列APIと対応main、SCCの構築補助処理を採用しました。変更後3件のFable再検討は3/3件完了し、`post_resubmission_review`で現在版SHAと次段階を管理します。旧GraphのAC・SHA・receiptは`historical`に残し、変更後ソースへ引き継ぎません。他のFastest候補は未採用です。

11月13日19:40（日本時間）の通常リセット後、5時間枠0%・週49%を確認してFable 5.2を再開しました。これは再開時点の値です。追加課金・無料リセット・別アカウント・別モデルは使っていません。Proの期間限定枠は11月17日16:59までです。

完了判定には現在の生成ソースSHAに対応するstage receiptと検証・測定根拠を使います。`is_error=true`の実行は、`subtype=success`でも完了扱いにしません。完了後のコード変更は再検討対象となります。

旧提出版までの通常版229件・生成版156提出コード256サンプルの検証記録と、変更後Graph3の独立照合・生成版検証は、それぞれ下記のAuditへ保存しています。本体・main・共通入出力・コンパイラ設定の効果を区別しています。

AtCoder7件の画像サイズは提出用ZIPのCRLF換算サイズと一致しますが、それだけで全文一致や現在版ACとは扱いません。

[件別の現在版SHA・履歴・次段階](remaining-2026-11-13.json) · [当初101件の記録](../../Audit/remaining-library-optimization-2026-11-13.json) · [Graph3再検討記録](../../Audit/fastest-reconsideration-codex-2026-11-13.json)
