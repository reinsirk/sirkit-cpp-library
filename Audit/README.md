# 開発の検証記録

測定条件、検証結果、出典を確認するための記録です。公開APIの使い方は[ライブラリ文書](../Document/README.md)を参照してください。

| フォルダ | 内容 |
| --- | --- |
| [Performance](Performance/) | 高速化の比較、CPU・コンパイラ条件、採用判断 |
| [Submissions](Submissions/) | オンライン判定、提出ソースの照合、結果画像 |
| [Verification](Verification/) | 正常系・失敗系の検証、Verify先の調査、CI・カバレッジ |
| [Provenance](Provenance/) | 原典の調査、著作者、ライセンスの根拠 |
| [Distribution](Distribution/) | 配布物の構成と配布前の検査 |
| [Changes](Changes/) | 過去の変更記録。新しい変更概要はコミットメッセージに記載 |
| [Benchmarks](Benchmarks/) | 性能測定用のプログラムと関連資料 |
| [Measurements](Measurements/) | 個別ベンチマークの測定データ |
| [data](data/) | 記録から参照する補足資料。検証ZIPはローカルに保存 |

提出資料はVerify IDごとの最新1件を保存します。性能記録は同じライブラリ・操作・比較範囲ごとに最新の測定に更新します。現在の判定やSHA照合に使う根拠は保持します。保存範囲は[提出記録](../Document/contributing/verification.md#提出コードと判定履歴)と[性能記録](../Document/contributing/performance.md#測定条件と判定)を参照してください。

過去のJSONには記録時点のパスが含まれます。移動したファイルは同じファイル名で用途別のフォルダから探せます。削除したファイルはGitの履歴から確認できます。[最適化の完了記録](../Verify/optimization/README.md)は移動後も根拠ファイルのSHA-256を照合します。

新しい性能記録はケース別の計測値と再現手順を保存します。入力は生成器、ソースはコミットから取得します。[測定データの保存](../Document/contributing/performance.md#測定データの保存)

共有する根拠には、対象ライブラリと日付が分かるファイル名を付けてください。一時ログは `.sirkit-local/logs/`、公開しない実験資料は `.sirkit-local/experiments/` に保存します。
