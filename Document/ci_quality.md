# CIの品質検査

`main`向けPRでは毎回全件のローカルVerifyを実行し、オンラインACとは別に結果を記録します。各ジョブの結果はActionsのSummaryで確認できます。詳しいJSON、HTML、診断結果はArtifactに14日間保存します。

| 検査 | 対象・結果 |
| --- | --- |
| C++23 Verify | 全対象をGCC/Clang、通常/標準代替、入力検査有効/無効でビルド・照合する。 |
| 配布8版の検査 | 全ファイルの単独構文検査と、主要13ライブラリの動作テストを行う。共通版はGCC・Clang・MSVCで実行する。 |
| 正常系・失敗系 | checked契約テストで正常結果・期待した例外型・失敗後の状態を照合する。 |
| Library Checker公式ケース | 全LC対象を外部キャッシュで生成し、全ケースを公式checkerで照合する。 |
| 決定的Max・構造ケース | Unionfindと永続UFのNo.2295向けに、制約上限や平衡結合・版探索・大きな積を独立参照と照合する。 |
| ASan/UBSan | 通常版のchecked実行で、範囲外アクセス・解放後の参照・未定義動作を検出する。 |
| カバレッジ | checked実行で通常版本体の行・直接分岐を測定し、両方100%を必須にする。未計測も失敗。 |
| 作法 | 通常版をCore Guidelines・Google Styleの2プロファイルで検査する。`Fast/`は対象外。 |
| 性能退行 | 通常版全103本のVerify全体時間と代表3本の本体時間を、PRのベースコミットと比較する。 |

各ライブラリの結果は[ライブラリページ](https://reinsirk.github.io/sirkit-cpp-library/)にも表示します。ソースSHAが合わない記録は未検証、対象のない項目は未実施です。[生成・公開・外部問題資産の扱い](library_pages.md)を参照してください。

```sh
python3 Scripts/ci_contracts.py --compiler g++ --output /tmp/contracts
python3 Scripts/ci_library_checker.py --all --base origin/main \
  --cache /tmp/library-checker-cache --output /tmp/official-cases
python3 Scripts/ci_quality_report.py --input /tmp/contracts --input /tmp/official-cases --output /tmp/quality
```

失敗系の例外は「期待した失敗を正しく処理した」ことを表します。異常終了・例外型の不一致・空の結果を合格にはしません。公式ケースの時間は起動・入出力を含むプログラム全体で、本体だけの代表性能とは別です。

## コンパイラとコンパイル引数

Linuxの一般設定はUbuntu 26.04でGCC 15（`g++-15`）とClang 18（`clang++-18`）を使います。配布版のMSVC検査はWindowsで実行します。

各検査ジョブは同じPRのソースを取得して並行に実行します。ローカルVerify、Linuxの配布版検査、ASan/UBSan、カバレッジ、Clang-Tidyでは、各ジョブ内でも最大4件を並行実行します。性能測定は各ランナーで順番に実行します。PRの最終判定は、生成物の同期検査を含む全ジョブの完了後に行います。

一般設定では`-std=c++23`を指定します。以下はジョブごとの主な追加引数です。

| ジョブ | 引数 |
| --- | --- |
| ローカルVerify | `-O2 -Wall -Wextra`。検査有効版と検査省略版をそれぞれ生成して実行。 |
| 配布版の動作テスト | `-O2 -UNDEBUG`。配布版と検査設定に応じて`SKIP_RUNTIME_CHECKS`を指定。 |
| 正常系・失敗系 | `-O2 -UNDEBUG -USKIP_RUNTIME_CHECKS` |
| Library Checker公式ケース | `-O2 -DSKIP_RUNTIME_CHECKS= -DNDEBUG=` |
| Verify全体の性能比較・Max検査 | `-O2 -DNDEBUG`。生成ソースで`SKIP_RUNTIME_CHECKS`を定義。 |
| ライブラリ本体の性能比較 | `-O2 -DNDEBUG -DSKIP_RUNTIME_CHECKS` |
| ASan/UBSan | `-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer` |
| カバレッジ | Clangは`-O0 -g -fprofile-instr-generate -fcoverage-mapping`。GCCの補測は`-O0 -g --coverage`。 |

標準代替のVerifyと契約テストは、`-U__SIZEOF_INT128__ -DSIRKIT_FORCE_STANDARD_ARITHMETIC -DSIRKIT_LINE_CONTAINER_FORCE_PORTABLE -DSIRKIT_DISABLE_SIMD`を追加します。計装の主計測では`-UNDEBUG -USKIP_RUNTIME_CHECKS`も指定します。

Verifyごとに設定されたGCC用の`#pragma GCC optimize`は生成ソースに入ります。該当するプログラムでは、コマンドラインの`-O2`に加えてこの設定が適用されます。性能比較の変更前後は同じGCC 15とコンパイル引数でビルドします。

### AtCoder設定

ローカルVerify全件をGCC・Clangの一般設定とAtCoder設定で実行します。Verify全体と代表3本の本体の性能比較はGCCの両設定で実行します。

- GCCの`atcoder`はGCC 15を使い、`-fmodules`とGCC用のconstexpr上限を指定します。`-flto=auto`は2026年6月の公式更新でGCC設定から削除されました。[更新のお知らせ](https://atcoder.jp/posts/language_update20260616_jp)
- Clangの`atcoder-clang`はUbuntuのClang 21とlibc++を使い、`-fconstexpr-steps=524288`、`-flto=auto`、`lld`、`compiler-rt`、`libunwind`を指定します。Verify全件を検査有効・省略の両方で実行します。[AtCoderの言語設定](https://img.atcoder.jp/file/language-update/2025-10/language-list.html)

両設定で`-std=gnu++23 -O2 -Wall -Wextra -march=native`、`ATCODER`・`ONLINE_JUDGE`・`NOMINMAX`、自動変数のゼロ初期化、OpenMPなどを指定します。実行環境はUbuntuのCIランナーです。AtCoder固有の検索パス、Verifyで使わない外部ライブラリやプリコンパイル済み標準モジュールは省きます。

性能退行は各設定内でPRの変更前後を比較します。判定はActionsのSummaryに表示します。AtCoder設定の引数と測定値は`atcoder-`で始まるArtifactで確認できます。ライブラリページには一般設定の結果を掲載します。

旧版だけがAtCoderの`constexpr`演算上限を超える場合、その設定では現行版の正解照合と計測を行い、退行判定は「比較不可」と記録します。一般設定の退行判定は通常どおり行います。

```sh
python3 Scripts/ci_verify.py --compiler g++-15 --compile-profile atcoder --id fenwick_tree
python3 Scripts/ci_verify.py --compiler clang++-21 --compile-profile atcoder-clang --id fenwick_tree
```

## 配布版の構文・動作検査

各処理系で共通版と専用版を検査します。配布用の通常版は入力検査を有効にした場合と省略した場合の両方で、速度優先版は配布時の設定で実行します。専用の速度優先実装を持つ11本すべてを動作テストに含め、独立した参照実装の結果と照合します。無効な入力を使う失敗系のテストは、入力検査を有効にした版で実行します。

`variant-builds-GCC`、`variant-builds-Clang`、`variant-builds-MSVC`に原本・生成物・動作テストのSHAと構成別の判定を保存します。MSVCはWindowsでVisual Studioの`cl.exe`を使います。ACL入りのテンプレートには`Verify/current/vendor/ACL_COMMIT`で固定したACLを使います。

```sh
python3 Scripts/verify_variants.py --compiler GCC --compiler Clang --syntax-only --run-tests \
  --acl-include /path/to/ac-library --output /tmp/variant-builds.json
```

Visual Studioの開発者コマンドプロンプトでは`--compiler MSVC`を指定し、出力先をWindowsのパスに置き換えます。

## Max・難ケースの範囲

公式データ生成だけは子プロセスのスタックを512MiB以上（環境の上限内）にし、公式正解の深い再帰に対応します。提出プログラムの実行条件は変更しません。生成失敗時は終了コード・シグナル・ログSHAを記録し、生ログや公式入力は成果物へ含めません。

毎PRでLC対象全件の公式ケースを実行し、`info.toml`の件数と入出力の集合・SHAを確認します。最大ランダムや専用TLE対策を名前で除外せず、最後のケースまで必要です。ただし`max_random`という名前だけでN・Qの両方が最大とは判断しません。

別のstressジョブでUnionfindのN=Q=200000と圧縮前の平衡木、No.2295のN最大200000・Q=300000の成分結合・版探索・64bitを超える中間積などを補います。巨大入力はリポジトリへ保存せず、その場で生成し、全出力を閉形式の独立参照と照合します。結果はActionsのSummaryと`stress-cases`成果物で確認できます。最大ケースはGCCで1回の専用検査とし、全コンパイラ・Sanitizerへ重複させません。

通常のVerify入力例だけでは大規模入力のMaxを保証しません。LC以外の問題Max補足は現在No.2295が対象で、他のyukicoder・AtCoder問題のMaxまで網羅したとは扱いません。公式runnerの1ケース30秒はCIの実行予算で、公式制限時間内の合格とは別です。

## ASan/UBSanとカバレッジ

```sh
python3 Scripts/ci_instrumented.py --mode sanitizers --compiler clang++-18 --output /tmp/sanitizers
python3 Scripts/ci_instrumented.py --mode coverage --compiler clang++-18 \
  --llvm-profdata llvm-profdata-18 --llvm-cov llvm-cov-18 \
  --gcc g++-15 --gcov gcov-15 --output /tmp/coverage
```

`--id`で個別指定できます。指定を省くとオンライン向けVerifyと独立参照を含むローカル補足の全件です。入力検査とassertを有効にし、失敗や結果の欠落はCI失敗にします。

カバレッジはプログラム別に計測し、原本の行・分岐位置ごとに到達結果をまとめます。分岐は原本に直接書かれた条件のtrue/falseを数え、マクロ展開内部を除きます。driver・標準ライブラリ・vendor・生成された派生版も除きます。入力例だけで全入力を網羅したとは扱いません。実体化されていないテンプレートや、テストから呼ばれていないAPIには、行カバレッジだけでは検出できない不足があります。

ファイル別と関数別のLLVM計測を原本座標で照合します。全ビルド・全実体化でコンパイル時に確定した条件は実行時分岐を持たないため、座標と元の計測値をHTMLとJSONに残し、分岐件数には含めません。いずれかのビルドに実行時分岐があれば、実行回数が両側とも0でも計測件数に含めます。

`gcc_line_tests`に登録したテストは、同じ原本と入力をGCCでも検査有効で実行します。LLVMが計測した行で`gcov`に正の実行回数がある場合、行の到達に反映します。分母と分岐の計測値はLLVMの出力を使います。補った行と実行回数は原本別のHTMLとJSONで確認できます。

`gcc_unchecked_line_tests`では、登録した単独テストを`SKIP_RUNTIME_CHECKS`でも実行します。テスト側のassertは有効のまま、有効な入力の独立照合に成功した場合だけ正の行カウンタを反映します。検査有効の主計測は必須で、補った行はHTMLとJSONに計測設定を記録します。

`clang_unchecked_tests`では、検査有効版に加え、ライブラリの検査を省略したClang版を別の実行ファイルとして測定します。両方の実行とプロファイル生成を必須とし、テストのassertを有効にしたまま、原本の行・分岐位置ごとに実測結果を合わせます。

`x86_baseline_tests`に登録したテストは、同じ計装バイナリをAVX2・POPCNTのないCPU設定のQEMUでも実行します。CPU機能の実行時判定から代替経路に入った回数を、ネイティブ実行のLLVMプロファイルと併合します。両方の実行・出力照合・プロファイル生成を必須にします。ローカルでの実行にも`qemu-x86_64`が必要です。

`x86_avx2_tests`に登録したテストは、検査有効のClangで`-mavx2`を指定した版もビルドし、AVX2対応のCPU設定のQEMUで実行します。コンパイル時にAVX2を選択する経路の実測を主計測に加えます。

`x86_binary128_tests`に登録したテストは、検査有効のClangで`-mlong-double-128`を指定した版も実行します。線分の判定で、113ビットの仮数の途中に0の桁が入る入力を実測します。テストで使う数学関数は、glibcのbinary128用関数を呼び出します。

`i386_tests`に登録したテストはClangの`-m32`で別途ビルドし、`qemu-i386`で実行します。LCAの祖先ラダー・祖先表、グラフのCSR、篩の表の容量検査などを32ビットの上限で測定し、64ビットの結果と同じ原本座標へ集計します。64ビット版と32ビット版の生プロファイルはそれぞれ対応するバイナリから出力します。Ubuntu 26.04では`g++-15-multilib`と`qemu-user`が必要です。

Dijkstraの距離上限を越える経路は、同じ内部テンプレートの上限を64とした小さいグラフで測定します。Bellman–Fordの独立実装で距離・経路・例外を照合します。公開APIは既定上限`2^60`を使い、その上限での頂点数の切り替え境界も配列を確保せず検査します。

合格条件は行・分岐とも100%です。到達件数と計測件数が一致するかで判定するため、百分率の丸めは判定に影響しません。選択した原本の未計測、計測行が0行の原本、翻訳単位の計測漏れも不合格です。分岐を持たない原本は、計測した行が1行以上あり、すべての行に到達した場合に分岐条件を満たします。実行の成否とカバレッジの判定は別に記録し、カバレッジが不足した場合も、件数とHTML・JSONを残します。ライブラリページも同じ条件で表示し、旧形式のPASSを100%到達と読み替えません。

GitHub Actionsでは`--jobs 4`でカバレッジのテストを4件ずつ並行実行します。入力ごとの実行上限は240秒です。原本ごとに行と各条件のtrue/false到達をすべて合算し、全体で100%を判定します。ローカルでは`--jobs`で同時実行数を指定できます。

AVX2・POPCNTを無効にしたQEMUの補足テストでは、公式アーカイブから取得した汎用AMD64のヘッダー・起動用オブジェクト・計装ライブラリでビルドし、汎用AMD64の動的ローダーとC/C++ランタイムで実行します。Ubuntu 26.04のクラウドイメージに含まれるAMD64v3の起動用オブジェクトを使うと、そのCPU要件が実行ファイルにも入ります。[Ubuntu 26.04のリリースノート](https://documentation.ubuntu.com/release-notes/26.04/summary-for-lts-users/#cloud)

役割のない到達不能コードは削除します。コンパイラや設定によって実行される箇所は、対応する構成で測定します。公開APIの前提と実装の不変条件から到達不能と証明した計測位置は、理由を添えて[除外記録](../Verify/ci/coverage_exclusions.json)へ登録し、分母から引きます。登録は原本のSHA-256、計測位置、証明の理由を持ちます。原本のSHAが変わった場合や登録箇所に実行が到達した場合はCIが失敗します。計測結果に該当位置がなければ分母は変わりません。除外前の計測値はJSON、除外位置と理由は原本別のHTMLとJSONに残します。

カバレッジでは、Verifyの登録入力に加え、[登録済みの単独テスト](../Verify/ci/coverage_tests.json)と代表6本のAPI契約テストを計測します。単独テストでは独立参照、境界値、期待した例外、確保失敗後の状態などを照合します。これらのテストは提出用コードの一覧から独立しています。`instrumented-coverage/html/index.html` に原本ごとの率と到達件数を表示し、未到達があるセルを赤く表示します。詳細ページから最初の未到達行・分岐に移動できます。公式全ケースのジョブは別に実行します。

## C++の作法

参照は[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)と[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)、検査には[clang-tidy](https://clang.llvm.org/extra/clang-tidy/)を使います。ルールはルートの`.clang-tidy`で管理し、通常版の本体をVerifyの呼び出しで実体化して調べます。

```sh
python3 Scripts/ci_cpp_guidelines.py --guide core --clang-tidy clang-tidy-18 --output /tmp/core
python3 Scripts/ci_cpp_guidelines.py --guide google --clang-tidy clang-tidy-18 --output /tmp/google
```

所有権・スライシング・仮想デストラクタ・move後の使用など、重大な項目を必須検査にします。初期化・コピー・キャストの改善候補も表示します。形式規則との衝突は[方針の一覧](cpp_style_policy.md)へ記録し、採否を相談します。自動検査で両ガイド全体への準拠を保証しません。`Fast/`と`Variants/`、vendor、連結提出コードは作法の診断対象外です。

確認済みの誤検出は`Scripts/ci_cpp_guidelines_exceptions.json`に理由・位置・診断・原本SHAを限定して保存し、レポートからは消しません。原本や診断が変わると例外は失効します。重大な規則が設定から抜けた場合も検査を失敗させます。

## 性能退行の検査

全103ライブラリについて、固定したVerifyプログラムの実行時間を8分割で測ります。テンプレート・`Fast/`・生成された派生版はこの103本に含めません。Library Checker対応の83本は公式入力の小規模例・最大バイト数・全generator系列・専用難ケースから選び、残り20本には各2形状の決定的入力を用意します。公式全ケースの正誤照合は別ジョブでも行います。

```sh
python3 Scripts/ci_performance_all.py --base origin/main --compiler g++ \
  --cache /tmp/all-performance-cache --output /tmp/all-performance
```

ActionsではPRのベースSHAを固定し、変更前後を同じジョブで、同じ入力・コンパイラ・設定で交互に測ります。Verifyは原則として同じmainと入出力を使います。旧APIでのみビルドできる場合は、同一問題・入出力で照合した旧driverを使い、JSONに出典とSHAを記録します。測定の既定はウォームアップ1回・測定3回です。Verify全体では起動・入出力を含むwall時間と子プロセスCPU時間を記録し、入力生成・コンパイル・正解checkerの時間は除きます。

非LCの規模はCI用の処理量で、問題Maxの網羅を意味しません。数値制約を未再確認の4件はJSONへその旨を記し、既存driverを使うAPI測定として扱います。呼び出していない公開APIの性能まで保証しません。現在版のライブラリ・ケース・測定値が欠ければ失敗とし、縮小確認は全件測定済みに数えません。

本体だけの補足測定には次を使います。入力生成・独立参照・起動・入出力は測らず、構築・操作・破棄・checksum集約を含めます。

```sh
python3 Scripts/ci_benchmark.py --base origin/main --compiler g++ --output /tmp/kernel-performance
```

代表3本の各14ケース、計42ケースを実行します。小規模・大規模・2の冪の直前直後に加え、200万要素をCI用stress上限とします。この値はAPIやオンライン問題の最大制約ではありません。

- Fenwick Tree：空区間・全区間・端点・立っているbitの多いprefix、正負の交互更新。
- UnionFind：ランダム・逐次結合、平衡結合後の圧縮前探索、未連結成分・重複結合。
- RollingHash：ランダム・同一文字、末尾だけ違う長いLCP、NUL・上位bitを含むバイト列と端点。

`--quick`は小規模なローカル確認用です。

Verify全体は子プロセスのCPU時間、本体はテスト用プログラム内の経過時間を使います。ライブラリとコンパイル設定の組み合わせごとに、比較可能な全ケースで反復測定の中央値を求めます。最も遅いケースが両版で異なる場合も、基準版と現行版それぞれの中央値の最大値を比べます。

上限は基準版の最大値に、割合と最小許容幅の大きい方を加えた値です。GitHub Actionsの標準ランナーでは`--regression-profile github-hosted`を使い、15%を許容します。短時間の測定に対する最小許容幅はVerify全体で1 ms、本体で50 µsです。ローカルの既定は5%で、最小許容幅はVerify全体で100 µs、本体で5 µsです。現行版の最大値が上限を超えたときは、そのライブラリの比較対象をすべて再測定し、再測定でも上限を超えた場合に性能退行で失敗します。

ActionsのSummary・ライブラリページ・JSONに、両版の最悪ケースと判定を表示します。ケース別の測定値と比率は参考として記録します。比較元にコードがない場合や旧版の出力が独立参照と一致しない場合は、理由を記録して比較不能と表示します。現在版の誤答・ビルド失敗・計測漏れは失敗です。
