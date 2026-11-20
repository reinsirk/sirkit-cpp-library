# 手元でライブラリの性能を測る

手元でライブラリの操作ごとに性能を測れます。Verifyプログラム全体の時間を調べる手順も掲載しています。

## Google Benchmarkでライブラリ本体を測る

LinuxでCMake 3.24以降とC++23対応のC++コンパイラを用意し、リポジトリのルートで実行します。初回の設定で、固定したGoogle Benchmark 1.10.0を取得します。 [Google Benchmarkのライセンス](https://github.com/google/benchmark/blob/v1.10.0/LICENSE)

```sh
cmake -S Benchmarks -B .sirkit-local/benchmarks/lca -DCMAKE_BUILD_TYPE=Release
cmake --build .sirkit-local/benchmarks/lca --parallel --target sirkit_benchmarks
cpu=12
taskset -c "$cpu" .sirkit-local/benchmarks/lca/sirkit_benchmarks \
  --benchmark_min_warmup_time=0.1 --benchmark_repetitions=7 \
  --benchmark_context="logical_cpu=$cpu,revision=$(git rev-parse HEAD)" \
  --benchmark_out=.sirkit-local/benchmarks/lca/result.json \
  --benchmark_out_format=json
```

`12` は使う論理CPU番号に置き換えます。対象を絞るには `--benchmark_filter='LcaQuery/balanced'` のように指定します。登録ケースは `--benchmark_list_tests` で確認できます。

- **LcaBuild** は辺列からの構築1回を測ります。内部の確保を含み、構築後の破棄は計測外です。
- **LcaQuery** は構築済みの木に対して同じ照会列を繰り返し、1照会あたりの時間を表示します。最適化で `get(u, v)` の計算が省かれないよう、照会結果を加算します。

鎖と平衡二分木を4,096頂点と524,288頂点で測ります。辺と照会列の生成、独立参照との照合、入出力は計測外です。`result.json` には各回のCPU時間、中央値、コンパイラのバージョン、本体のSHA-256、入力検査の設定が入ります。

既定は入力検査ありです。検査を省略する測定では、別のビルドディレクトリを使い、CMake設定時に `-DSIRKIT_BENCHMARK_SKIP_RUNTIME_CHECKS=ON` を追加します。版を比較するときは、同じCPU、コンパイラ、ビルド設定、入力検査の設定をそろえます。

## Verifyプログラム全体を測る

LinuxでPython 3.11以降、Git、C++23対応のGCCを用意し、チェックアウトしたリポジトリのルートで実行します。次はFenwick Treeの小規模入力で動作を確認する例です。

```sh
python3 Scripts/create_performance_reference.py --compiler g++ \
  --library DataStructure/Fenwick_Tree.cpp --quick
```

代表的な規模で測るには `--quick` を外します。`--library` は複数指定でき、省略すると全ライブラリを測ります。

Library Checkerの通常規模のケースは、固定したリビジョンの公式データから生成します。公式データの取得時にはネットワーク接続が必要です。繰り返し測る場合は `--cache /tmp/sirkit-performance-cache` のようにリポジトリ外の保存先を指定すると、取得・生成したデータを再利用できます。

測定先を固定するには `--cpu` に論理CPU番号を指定します。再測定では同じCPU、コンパイラ、コンパイル設定を使います。AtCoderのGCC設定を使う場合は `--compile-profile atcoder` を追加します。

### 結果を見る

コマンドが表示する `.sirkit-local/performance/` 配下の保存先を開きます。

- `summary.md` に入力規模と時間の中央値を表示します。`CPU ms` はVerifyプログラムが使ったCPU時間、`wall ms` は起動と入出力を含む実行の経過時間です。
- `result.json` に各回の時間、測定条件、入力と出力のSHA-256を、`reference.json` にコミットSHAと実行オプションを保存します。

Verifyプログラム全体の改修前後を比較する手順は[性能退行の検査](ci_quality.md#性能退行の検査)を参照してください。
