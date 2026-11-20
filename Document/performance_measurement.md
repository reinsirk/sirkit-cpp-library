# 手元でライブラリの性能を測る

ライブラリを使ったVerifyプログラムを手元で実行し、入力ケースごとの時間を測れます。

## 測定する

LinuxでPython 3.11以降、Git、C++23対応のGCCを用意し、チェックアウトしたリポジトリのルートで実行します。次はFenwick Treeの小規模入力で動作を確認する例です。

```sh
python3 Scripts/create_performance_reference.py --compiler g++ \
  --library DataStructure/Fenwick_Tree.cpp --quick
```

代表的な規模で測るには `--quick` を外します。`--library` は複数指定でき、省略すると全ライブラリを測ります。

Library Checkerの通常規模のケースは、固定したリビジョンの公式データから生成します。公式データの取得時にはネットワーク接続が必要です。繰り返し測る場合は `--cache /tmp/sirkit-performance-cache` のようにリポジトリ外の保存先を指定すると、取得・生成したデータを再利用できます。

測定先を固定するには `--cpu` に論理CPU番号を指定します。再測定では同じCPU、コンパイラ、コンパイル設定を使います。AtCoderのGCC設定を使う場合は `--compile-profile atcoder` を追加します。

## 結果を見る

コマンドが表示する `.sirkit-local/performance/` 配下の保存先を開きます。

- `summary.md` に入力規模と時間の中央値を表示します。`CPU ms` はVerifyプログラムが使ったCPU時間、`wall ms` は起動と入出力を含む実行の経過時間です。
- `result.json` に各回の時間、測定条件、入力と出力のSHA-256を、`reference.json` にコミットSHAと実行オプションを保存します。

時間は手元の環境でのVerifyプログラム全体の値です。個別のライブラリ操作だけを測る場合や、改修前後を比較する場合は[性能退行の検査](ci_quality.md#性能退行の検査)を参照してください。
