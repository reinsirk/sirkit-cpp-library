# 開発とVerify

開発は`dev`で行い、まとまった変更をGitHubのPRで`main`へ反映します。push・PR作成は手動で行います。未コミットの変更はブランチ切り替えだけではコミットされません。

配布は[GitHub Releases](https://github.com/reinsirk/sirkit-cpp-library/releases)を使います。配布ZIPとその管理用manifest・検査結果はリポジトリに置かず、通常の変更時には再生成しません。本体・使用例・Verifyなどの生成物の同期確認は引き続き行います。

## 最適化

対象は[Verify状況の一覧](../Verify/current/status.md)から選びます。Library Checkerの提出ではライブラリ本体、main、入出力を含めて検証し、本体と提出プログラム全体の性能を別々に測ります。その他の提出先では正しさを検証し、高速化はライブラリ本体を対象とします。

```sh
python3 Scripts/prepare_verify.py
python3 Scripts/prepare_variant_verify.py
```

通常は共通の高速入出力を使います。独自の入出力が有効な場合は、`Verify/current/groups/*.json`のLibrary Checker対象項目に`allow_custom_io: true`を指定できます。main全体が変更の追跡対象です。yukicoder・AtCoder・AOJでは素直なAPI呼び出しを使い、問題の制約内で解くために必要な手順を記します。yukicoder・AtCoderの対話問題では `interactive_transcript: true` を指定し、問い合わせを都度フラッシュする入出力と、そのドライバーで再現できる応答例を用意します。

提出版は`SKIP_RUNTIME_CHECKS`を有効にし、実行時assertを含む場合は`NDEBUG`も付けます。各オンライン対象の`gcc_optimization`には、測定で選んだ`O2`・`O3`・`O3,unroll-loops`を指定できます。Clangにはこのpragmaを適用しません。ローカルのchecked検証ではassertを有効に保ちます。

## ローカル確認とCI

C++の整形は原則として clang-format 21 とリポジトリの `.clang-format` に従います。波括弧や参照の書式は [template.cpp](../template.cpp) で確認できます。編集したソースを指定してください。

```sh
clang-format-21 -i template.cpp
```

生成済みの配布版、Verify提出ソース、同梱した外部コードは `.clang-format-ignore` で除外します。

```sh
python3 -m unittest discover -s Scripts/tests -p 'test_*.py'
python3 Scripts/ci_verify.py --id z_algorithm --compiler g++
python3 Scripts/ci_verify.py --base origin/main --list
```

`--id`で個別に指定し、`--base`で指定した版からの変更に関係するVerifyを選びます。全件を実行するには`--all`を使います。入力検査を有効にした場合と無効にした場合の両方で、ローカルの入出力例を照合します。Clangで標準C++の代替経路も確認するには`--compiler clang++ --fallback`を付けます。

[GitHub Actions](../.github/workflows/verify.yml)は、`main`向けPRの作成・更新時に毎回全件を実行します。`dev`や`main`へのpush単独では起動しません。

生成物・Pythonテスト・README集計を確認した後、GCC/Clangそれぞれで通常経路と標準C++の代替経路を実行します。各経路で入力検査有効・無効の両方を照合します。対象なしは「対象なし」と表示し、未実行のコードを検証済みとは数えません。

README先頭のバッジは`main`向けPRのCI結果です。PRごとの結果はChecksの`Verify result`、コンパイラごとの件数・成否はActionsのSummary、詳しいJSONは14日間保存するArtifactで確認できます。GitHubのmain保護ルールで`Verify result`を必須にすると、いずれかの検査が失敗したPRを取り込まない運用にできます。オンライン提出とAC判定は別途行います。

ASan/UBSan、カバレッジ、通常版のCore Guidelines・Google Style検査も実行します。対象と結果の読み方、ローカルでの実行方法は[CIの品質検査](ci_quality.md)を参照してください。性能の比較手順は[手元でライブラリの性能を測る](performance_measurement.md)にまとめています。

## 手動提出と判定の取り込み

提出先はLibrary Checker→yukicoder→AtCoder→AOJの順です。問題を移した場合は旧問題・判定を参考履歴として残し、新問題のACは別に取得します。説明コメントのみの変更8件の再提出免除は[提出方針](../Verify/current/submission_policy.json)のソースSHAに限定し、実装変更後へ引き継ぎません。

`Verify/current/submissions/ID.cpp`を提出します。変更後のコードでACを得たら、提出URLから実際の本文を照合して記録します。

```sh
python3 Scripts/record_library_checker_submission.py \
  --id z_algorithm --url https://judge.yosupo.jp/submission/提出番号
# 表示を確認した後、同じ指定に --write を付けて保存する
python3 Scripts/update_verify_status.py
```

取り込みには公開APIを使い、ログイン情報は不要です。既定では確認だけで、`--write`を付けた場合だけ以前の記録を履歴に残して保存します。提出先・実際の提出本文・判定を確認できない場合は保存しません。

READMEの「現行AC」は、生成するコードと保存済みの受理コードを照合できた件数です。過去のAC、変更後の要再提出、未提出、ローカル補足を区別します。集計の更新忘れは`python3 Scripts/update_verify_status.py --check`で確認できます。
