# CIの品質検査

`main`向けPRでは毎回全件のローカルVerifyを実行します。オンラインACとは別の結果です。各ジョブの結果はActionsのSummary、詳しいJSON・HTML・診断は14日間保存するArtifactで確認できます。

| 検査 | 対象・結果 |
| --- | --- |
| C++23 Verify | 全対象をGCC/Clang、通常/標準代替、入力検査有効/無効でビルド・照合する。 |
| ASan/UBSan | 通常版のchecked実行で、範囲外アクセス・解放後の参照・未定義動作を検出する。 |
| カバレッジ | checked実行で通常版本体の行・直接分岐の到達状況をHTML/JSONへ出す。 |
| 作法 | 通常版をCore Guidelines・Google Styleの2プロファイルで検査する。`Fast/`は対象外。 |
| 性能 | 固定入力の代表APIを、比較元と同じプログラム・コンパイラ設定で交互測定する。 |

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

```sh
python3 Scripts/ci_benchmark.py --base origin/main --compiler g++ --output /tmp/performance
```

`main`向けPRの取り込み先の版と比較します。比較元の本体だけを取り出し、現在と同じ測定プログラムを使います。旧global版と現行sirkit版の両方に対応します。代表APIの本体測定であり、Verifyの入出力を含む時間やLibrary Checkerの提出時間とは別です。

入力生成・正しさの照合を測定区間と分け、ウォームアップ後に前後を交互に実行して中央値と生データを保存します。速度差だけではPRを落としませんが、ビルド失敗・誤答・計測失敗は落とします。比較元に対象がない初回は現在版だけを測り「比較元なし」と表示します。
