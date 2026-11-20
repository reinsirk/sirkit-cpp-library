# sirkit-cpp-library

[![ライセンス CC0 1.0](https://img.shields.io/badge/license-CC0%201.0-blue)](#license)
[![最新リリース](https://img.shields.io/github/v/release/reinsirk/sirkit-cpp-library?display_name=tag&sort=date)](https://github.com/reinsirk/sirkit-cpp-library/releases/latest)
[![main向けPRのCI](https://github.com/reinsirk/sirkit-cpp-library/actions/workflows/verify.yml/badge.svg?event=pull_request)](https://github.com/reinsirk/sirkit-cpp-library/actions/workflows/verify.yml?query=event%3Apull_request)

競技プログラミング向けのC++ライブラリ集です。

[ライブラリページ](https://reinsirk.github.io/sirkit-cpp-library/)でAPI、使用例、ソース、テスト結果を確認できます。[ページの生成と公開](Document/library_pages.md)を参照してください。

自分の環境で測るには[手元でライブラリの性能を測る](Document/performance_measurement.md)を参照してください。

CIバッジはVerify、ASan/UBSan、カバレッジ、通常版の作法検査などの結果を示します。検証件数とカバレッジ率はChecksやActionsのSummaryで、オンラインACは下の表で確認できます。検査の対象と判定基準は[品質検査の説明](Document/ci_quality.md)を参照してください。

<!-- VERIFY_STATUS:START -->
[Verify状況の一覧](Verify/current/status.md)（通常版の現行ソースと保存した判定を照合）

| 提出先 | 対象 | 現行AC | 現行未照合 | 未提出 | 非AC |
| --- | ---: | ---: | ---: | ---: | ---: |
| Library Checker | 145 | 13 | 88 | 44 | 0 |
| AOJ | 0 | 0 | 0 | 0 | 0 |
| Yukicoder | 34 | 0 | 2 | 32 | 0 |
| AtCoder | 52 | 0 | 34 | 18 | 0 |

説明変更のみの再提出免除: 0件、移行前の参考検証: 17件。いずれも再提出対象から除き、現行ACへの新規照合とは区別します。

ローカル補足: 54件。過去のAC・ローカル結果は現行ACと区別します。
<!-- VERIFY_STATUS:END -->

各ソースの冒頭に、公開APIの引数・戻り値・計算量・前提をまとめています。詳しい説明は、API一覧の直上にあるDocumentへのリンクから参照できます。

公開APIは`sirkit`名前空間内です。競プロ用mainでは`using namespace sirkit;`、名前を明示したいコードでは`sirkit::segtree`等で利用します。共有の無限大定数は`sirkit::kInf`です。

## ライブラリの配置とVerify

| フォルダ | 内容 |
| --- | --- |
| [Graph](Graph/README.md) | 最短路・木・連結性・マッチング・フロー |
| [DataStructure](DataStructure/README.md) | 区間集計・動的列・ヒープ・Union-Find・ハッシュ表 |
| [Math](Math/README.md) | 分数・数論・畳み込み・多項式・線形代数 |
| [String](String/README.md) | ハッシュ・接尾辞・文字列照合・オートマトン |
| [Geometry](Geometry/README.md) | 点・直線／線分・多角形・円・最近点対・偏角ソート |
| [Algorithm](Algorithm/README.md) | LIS・Radix Sort・組合せ列挙・行列変換・Mo |
| [Internal](Internal/README.md) | 内部用の軽量ハッシュ表 |
| [Document](Document/README.md) | アルゴリズムの説明・計算量・利用前提 |

現在版の再Verifyは[Verify/current](Verify/current/README.md)で管理します。提出先・選定理由・ライブラリ込みのコード・ローカル検査結果をまとめています。
速度確認のため、外部提出用コードは先頭で`SKIP_RUNTIME_CHECKS`を定義しています。
直下の`Verify/*.md`は過去の提出記録です。今回のオンライン判定結果とは区別します。

Library Checkerの提出コードはmainと入出力も含めて最適化し、変更後に再提出します。ライブラリ本体とプログラム全体の速度は別々に比較します。開発は`dev`で行い、PRを通じて`main`に反映します。CI・手動提出・README集計の手順は[開発とVerify](Document/development.md)を参照してください。

使用例は[Examples](Examples)、二項係数のテーブル上限などの設定は[二項係数の説明](Math/Combination/README.md)を参照してください。

[通常版](Variants/README.md)と、競技プログラミングの提出向けの[速度優先版](Fast/README.md)に、それぞれコンパイラ共通・GCC専用・Clang専用・MSVC専用を用意しています。利用前提と生成方法は[配布版の説明](Document/fast_variants.md)を参照してください。

配布は[GitHub Releases](https://github.com/reinsirk/sirkit-cpp-library/releases)を利用します。

[連結ツール](Document/bundling.md)で必要なライブラリを読みやすい形で一つにまとめ、提出コードの容量を確認できます。

先頭で次を定義すると、入力検査をコンパイル時に省けます。

```cpp
#define SKIP_RUNTIME_CHECKS
```

## 開発支援

実装・最適化・検証・ドキュメントの整備にCodexとClaudeを利用しています。

## License

My original contributions to this repository are dedicated to the public domain under [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/).

Known third-party sources include CC0-1.0 code and the SplitMix64 finalizer. SplitMix64 is provided under its author's public-domain dedication and unconditional permission, a declaration distinct from CC0-1.0; see its [original notice](LICENSES/LicenseRef-SplitMix64-Public-Domain.txt).

The original public domain dedications and license terms remain applicable to those third-party works. I do not claim ownership of or purport to waive any rights held by third parties.

Bundled ACL code used by verification and examples is covered by [ACL's CC0 1.0 dedication](Verify/current/vendor/ACL_LICENSE).

Third-party source URLs are recorded in the corresponding source files, including for CC0-1.0 and public-domain code.

All code is provided without warranty, to the extent permitted by applicable law.
