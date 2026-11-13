# 開発とVerify

開発は`dev`で行い、まとまった変更をGitHubのPRで`main`へ反映します。push・PR作成は手動で行います。未コミットの変更はブランチ切り替えだけではコミットされません。

## 最適化

受理履歴のあるVerifyコードを[一覧](../Verify/optimization/README.md)から選びます。Library Checkerでは本体・main・入出力を含めてCodex→Claudeの順に改善し、各段階のコード・正しさの照合・測定結果を保存します。本体単独の比較と全体の比較は分けます。

```sh
python3 Scripts/optimization_handoff.py --list
python3 Scripts/prepare_verify.py
python3 Scripts/prepare_variant_verify.py
```

通常は共通の高速入出力を使います。独自の入出力が有効な場合は、`Verify/current/groups/*.json`のLibrary Checker対象項目に`allow_custom_io: true`を指定できます。main全体が変更の追跡対象です。AtCoderでは共通の高速入出力を使います。

提出版は`SKIP_RUNTIME_CHECKS`を有効にし、実行時assertを含む場合は`NDEBUG`も付けます。各オンライン対象の`gcc_optimization`には、測定で選んだ`O2`・`O3`・`O3,unroll-loops`を指定できます。Clangにはこのpragmaを適用しません。ローカルのchecked検証ではassertを有効に保ちます。

## ローカル確認とCI

```sh
python3 -m unittest discover -s Scripts/tests -p 'test_*.py'
python3 Scripts/ci_verify.py --id z_algorithm --compiler g++
python3 Scripts/ci_verify.py --base origin/main --list
```

`--id`は個別指定、`--base`は指定した版からの変更に対応するVerifyの選択、`--all`は全件です。検査有効・無効の両方でローカルの入出力例を照合します。Clangで標準C++の代替経路も確認するには`--compiler clang++ --fallback`を付けます。

[GitHub Actions](../.github/workflows/verify.yml)は`dev`へのpushと`main`向けPRで、生成物・Pythonテスト・README集計と、GCC/ClangのローカルVerifyを確認します。手動実行では全件を選べます。オンライン提出は別途行います。

## 手動提出と判定の取り込み

`Verify/current/submissions/ID.cpp`を提出します。変更後のコードでACを得たら、提出URLから実際の本文を照合して記録します。

```sh
python3 Scripts/record_library_checker_submission.py \
  --id z_algorithm --url https://judge.yosupo.jp/submission/提出番号
# 表示を確認した後、同じ指定に --write を付けて保存する
python3 Scripts/update_verify_status.py
```

取り込みには公開APIを使い、ログイン情報は不要です。既定では確認だけで、`--write`を付けた場合だけ以前の記録を履歴に残して保存します。提出先・実際の提出本文・判定を確認できない場合は保存しません。

READMEの「現行AC」は、生成するコードと保存済みの受理コードを照合できた件数です。過去のAC、変更後の要再提出、未提出、ローカル補足を区別します。集計の更新忘れは`python3 Scripts/update_verify_status.py --check`で確認できます。
