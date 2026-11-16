# CIの品質検査

`main`向けPRでは毎回全件のローカルVerifyを実行します。オンラインACとは別の結果です。各ジョブの結果はActionsのSummary、詳しいJSON・HTML・診断は14日間保存するArtifactで確認できます。

| 検査 | 対象・結果 |
| --- | --- |
| C++23 Verify | 全対象をGCC/Clang、通常/標準代替、入力検査有効/無効でビルド・照合する。 |
| 正常系・失敗系 | checked契約テストで正常結果・期待した例外型・失敗後の状態を照合する。 |
| Library Checker公式ケース | 全LC対象を外部キャッシュで生成し、全ケースを公式checkerで照合する。 |
| 決定的Max・構造ケース | Unionfindと永続UFのNo.2295向けに、制約上限や平衡結合・版探索・大きな積を独立参照と照合する。 |
| ASan/UBSan | 通常版のchecked実行で、範囲外アクセス・解放後の参照・未定義動作を検出する。 |
| カバレッジ | checked実行で通常版本体の行・直接分岐の到達状況をHTML/JSONへ出す。 |
| 作法 | 通常版をCore Guidelines・Google Styleの2プロファイルで検査する。`Fast/`は対象外。 |
| 性能 | 通常版全103本のVerify全体時間と、代表3本の本体時間を記録する。入力規模・CPU・コンパイラを併記する。 |

各ライブラリの結果は[ライブラリページ](https://reinsirk.github.io/sirkit-cpp-library/)にも表示します。ソースSHAが合わない記録は未検証、対象のない項目は未実施です。[生成・公開・外部問題資産の扱い](library_pages.md)を参照してください。

```sh
python3 Scripts/ci_contracts.py --compiler g++ --output /tmp/contracts
python3 Scripts/ci_library_checker.py --all --base origin/main \
  --cache /tmp/library-checker-cache --output /tmp/official-cases
python3 Scripts/ci_quality_report.py --input /tmp/contracts --input /tmp/official-cases --output /tmp/quality
```

失敗系の例外は「期待した失敗を正しく処理した」ことを表します。異常終了・例外型の不一致・空の結果を合格にはしません。公式ケースの時間は起動・入出力を含むプログラム全体で、本体だけの代表性能とは別です。

## Max・難ケースの範囲

毎PRでLC対象全件の公式ケースを実行し、`info.toml`の件数と入出力の集合・SHAを確認します。最大ランダムや専用TLE対策を名前で除外せず、最後のケースまで必要です。ただし`max_random`という名前だけでN・Qの両方が最大とは判断しません。

別のstressジョブでUnionfindのN=Q=200000と圧縮前の平衡木、No.2295のN最大200000・Q=300000の成分結合・版探索・64bitを超える中間積などを補います。巨大入力はリポジトリへ保存せず、その場で生成し、全出力を閉形式の独立参照と照合します。結果はActionsのSummaryと`stress-cases`成果物で確認できます。最大ケースはGCCで1回の専用検査とし、全コンパイラ・Sanitizerへ重複させません。

通常のVerify入力例だけでは大規模入力のMaxを保証しません。LC以外の問題Max補足は現在No.2295が対象で、他のyukicoder・AtCoder問題のMaxまで網羅したとは扱いません。公式runnerの1ケース30秒はCIの実行予算で、公式制限時間内の合格とは別です。

## ASan/UBSanとカバレッジ

```sh
python3 Scripts/ci_instrumented.py --mode sanitizers --compiler clang++-18 --output /tmp/sanitizers
python3 Scripts/ci_instrumented.py --mode coverage --compiler clang++-18 \
  --llvm-profdata llvm-profdata-18 --llvm-cov llvm-cov-18 --output /tmp/coverage
```

`--id`で個別指定できます。指定を省くとオンライン向けVerifyと独立参照を含むローカル補足の全件です。入力検査とassertを有効にし、失敗や結果の欠落はCI失敗にします。

カバレッジはプログラム別に計測し、原本の行・分岐位置ごとに到達結果をまとめます。分岐は原本に直接書かれた条件のtrue/falseを数え、マクロ展開内部を除きます。driver・標準ライブラリ・vendor・生成された派生版も除きます。入力例だけで全入力を網羅したとは扱いません。実体化されていないテンプレートや、テストから呼ばれていないAPIには、行カバレッジだけでは検出できない不足があります。数値の最低基準は設けず、未到達の分岐をテストへ追加するために使います。

## C++の作法

参照は[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)と[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)、検査には[clang-tidy](https://clang.llvm.org/extra/clang-tidy/)を使います。ルールはルートの`.clang-tidy`で管理し、通常版の本体をVerifyの呼び出しで実体化して調べます。

```sh
python3 Scripts/ci_cpp_guidelines.py --guide core --clang-tidy clang-tidy-18 --output /tmp/core
python3 Scripts/ci_cpp_guidelines.py --guide google --clang-tidy clang-tidy-18 --output /tmp/google
```

所有権・スライシング・仮想デストラクタ・move後の使用など、重大な項目を必須検査にします。初期化・コピー・キャストの改善候補も表示します。形式規則との衝突は[方針の一覧](cpp_style_policy.md)へ記録し、採否を相談します。自動検査で両ガイド全体への準拠を保証しません。`Fast/`と`Variants/`、vendor、連結提出コードは作法の診断対象外です。

確認済みの誤検出は`Scripts/ci_cpp_guidelines_exceptions.json`に理由・位置・診断・原本SHAを限定して保存し、レポートからは消しません。原本や診断が変わると例外は失効します。重大な規則が設定から抜けた場合も検査を失敗させます。

## 性能測定

全103ライブラリについて、固定したVerifyプログラムの実行時間を8分割で測ります。テンプレート・`Fast/`・生成された派生版はこの103本に含めません。Library Checker対応の83本は公式入力の小規模例・最大バイト数・全generator系列・専用難ケースから選び、残り20本には各2形状の決定的入力を用意します。公式全ケースの正誤照合は別ジョブでも行います。

```sh
python3 Scripts/ci_performance_all.py --compiler g++ \
  --cache /tmp/all-performance-cache --output /tmp/all-performance
```

ActionsのSummary・ライブラリページ・JSONに、ケース名、入力規模、実行時間の中央値と全測定値、CPU・コンパイラ・設定を残します。既定はウォームアップ1回・測定3回です。時間は起動・入出力を含むwall時間と子プロセスCPU時間を分け、入力生成・コンパイル・正解checkerの時間は除きます。Library Checker提出時間や本体だけの時間とは区別して参照してください。

非LCの規模はCI用の処理量で、問題Maxの網羅を意味しません。数値制約を未再確認の4件はJSONへその旨を記し、既存driverを使うAPI測定として扱います。呼び出していない公開APIの性能まで保証しません。現在版のライブラリ・ケース・測定値が欠ければ失敗とし、縮小確認は全件測定済みに数えません。

本体だけの補足測定には次を使います。入力生成・独立参照・起動・入出力は測らず、構築・操作・破棄・checksum集約を含めます。

```sh
python3 Scripts/ci_benchmark.py --compiler g++ --output /tmp/kernel-performance
```

代表3本の各14ケース、計42ケースを実行します。小規模・大規模・2の冪の直前直後に加え、200万要素をCI用stress上限とします。この値はAPIやオンライン問題の最大制約ではありません。

- Fenwick Tree：空区間・全区間・端点・立っているbitの多いprefix、正負の交互更新。
- UnionFind：ランダム・逐次結合、平衡結合後の圧縮前探索、未連結成分・重複結合。
- RollingHash：ランダム・同一文字、末尾だけ違う長いLCP、NUL・上位bitを含むバイト列と端点。

どちらも`--quick`は小規模の動作確認だけで、CIでは指定しません。比較元は不要ですが、必要なら`--base origin/main`で同じmain・入出力またはハーネスを使った比較もできます。速度の数値に合否基準は設けず、ビルド失敗・誤答・計測失敗を不合格にします。
