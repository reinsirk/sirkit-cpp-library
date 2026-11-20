# 最適化の引き継ぎ

`Scripts/optimization_handoff.py`は受理済みのVerifyコードをCodex・Claudeへ順に渡すための入力と記録を作ります。Library Checkerではmainと入出力を含む全体をCodex、Claudeの順で確認します。Library Checker以外は正しさの検証を主目的とし、最適化の対象はライブラリ本体に限ります。各段階の記録を、提出コード全文のSHA-256と測定・検証の根拠に結び付けます。

[PersistentUnionFindのAPI変更と追加Verify](persistent-uf-api-2026-11-16.md)も参照してください。[提出29本の結果とFastest比較](submissions29-results-2026-11-16.md)を更新しました。共有変更が波及した既存10本は[再確認結果](shared-followup-2026-11-15.md)にまとめています。今回追加した19本の結果と提出ソースは[追加LC Verifyの最適化](added-lc-optimization-2026-11-15.md)、全体の対象は[merge前の最適化](premerge-2026-11-14.md)を参照してください。[2026年11月13日の一覧](remaining-2026-11-13.md)は移行前の提出SHAに対する履歴です。

[残り10本の記録照合](remaining-review-reconciliation-2026-11-16.md)では、既存の実提出・実測・レビューから4本の完了記録を復元しています。残る6本は未完了です。

## 一覧と固定した入力

```sh
python3 Scripts/optimization_handoff.py --list
python3 Scripts/optimization_handoff.py --prepare z_algorithm --stage codex \
  --output-dir /tmp/z-codex
python3 Scripts/optimization_handoff.py --prepare z_algorithm --stage claude \
  --codex-receipt Audit/z-codex-stage.json --output-dir /tmp/z-claude
python3 Scripts/optimization_handoff.py --check-bundle /tmp/z-claude
```

既定の一覧は、現在または過去にACの記録がある問題だけです。現在のソースと過去のACが異なるものも含みますが、`current_ac=false`・`submission_required=true`として区別します。説明コメントのみの8件は指定SHAに限り再提出免除、移行前の参考検証は対象外とし、[提出方針](../current/submission_policy.json)で管理します。未受理の問題を含めるには`--include-unverified`、個別に始めるには`--prepare ID`を使います。

引き継ぎ先はリポジトリ外の新しいディレクトリにします。提出コード、本体、main、共通ヘッダー・ACL依存、AGENTS、各SHA、依頼文を保存します。元ソースは変更せず、既存ディレクトリも上書きしません。同じ本体を使うVerify IDもmanifestへ記録します。

## 段階の完了記録

Codexの後は、測定とローカル照合の根拠をファイルへ保存し、次の形式でreceiptを用意します。SHAは64桁の実値へ置き換えます。`source_sha256`は採用後の生成コード、`input_source_sha256`は着手前のコードです。

```json
{
  "format": 1,
  "id": "z_algorithm",
  "stage": "codex",
  "status": "complete",
  "input_source_sha256": "着手前の全文SHA-256",
  "source_sha256": "採用後の全文SHA-256",
  "changed": true,
  "scope": "whole_program",
  "checks": {"correctness": "passed", "performance": "measured"},
  "evidence": [{"path": "Audit/z-measurement.json", "sha256": "根拠ファイルのSHA-256"}]
}
```

変更なしでも、正しさと性能を確認して`changed=false`・同じ入出力SHAで記録できます。新規の記録はLibrary Checkerで`scope=whole_program`、それ以外で`scope=library`とします。過去の記録は当時の範囲として保持します。根拠の要約JSONは`Audit/`へ置き、詳細な検証ZIPは`Audit/data/`へローカル保存します。ZIPはGitで追跡しないため、別の環境で履歴を照合する場合は必要なZIPも別途コピーしてください。不足時は照合を失敗させ、確認済みとは扱いません。

```sh
python3 Scripts/optimization_handoff.py --record-stage Audit/z-codex-stage.json
python3 Scripts/optimization_handoff.py --record-stage Audit/z-claude-stage.json \
  --codex-receipt Audit/z-codex-stage.json
```

Claudeのreceiptは`stage=claude`とし、入力SHAをCodexの出力SHAに合わせます。完了記録は`Verify/optimization/stages/`へ内容ハッシュ付きの名前で保存し、上書きしません。根拠が変わった記録や、現在のソースと一致しない完了記録は受け付けません。

Claudeの確認まで終えても、オンラインでACになったコードと変更後のコードが一致するまでは、一覧の`next_stage`はresubmitです。本体を変えたら同じ本体を含む提出コードも再生成・照合・再提出し、mainや入出力の変更を本体単独の成果に含めません。このスクリプトは提出やClaudeの実行を自動では行いません。

提出URLコメント等だけが変わった場合は、現在のACと厳密に照合できた受理ソースのSHAだけを同一版として扱います。current_submission_sha256等の現在版メモだけでは、完了を引き継ぎません。
