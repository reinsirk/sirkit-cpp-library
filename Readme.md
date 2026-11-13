# sirkit-cpp-library

競技プログラミング向けのC++ライブラリ集です。

<!-- VERIFY_STATUS:START -->
[Verify状況の一覧](Verify/current/status.md)（通常版の現行ソースと保存した判定を照合）

| 提出先 | 対象 | 現行AC | 現行未照合・要再提出 | 未提出 | 非AC |
| --- | ---: | ---: | ---: | ---: | ---: |
| Library Checker | 125 | 4 | 64 | 57 | 0 |
| AtCoder | 50 | 0 | 34 | 16 | 0 |

ローカル補足: 54件。過去のAC・ローカル結果は現行ACと区別します。
<!-- VERIFY_STATUS:END -->

各ソースの冒頭に、公開APIの引数・戻り値・計算量・前提をまとめています。詳しい説明は、その直上にあるDocumentへのリンクから参照できます。

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

Library Checkerの提出コードはmain・入出力を含めてCodex、Claudeの順に最適化し、変更後は再提出します。本体だけの速度比較と、プログラム全体の比較は分けて記録します。開発はdev、mainへの反映はPR経由で行います。CI・手動提出・README集計の操作は[開発とVerify](Document/development.md)を参照してください。

使用例は[Examples](Examples)、二項係数のテーブル上限などの設定は[二項係数の説明](Math/Combination/README.md)を参照してください。

[コンパイラ別版](Variants/README.md)と[速度優先版](Fast/README.md)も用意しています。通常版との違い・利用前提・生成方法は[説明](Document/fast_variants.md)を参照してください。

必要なライブラリの読みやすい連結と、提出コードの容量確認には[連結ツール](Document/bundling.md)を使えます。

先頭で次を定義すると、入力検査をコンパイル時に省けます。

```cpp
#define SKIP_RUNTIME_CHECKS
```

## License

My original contributions to this repository are dedicated to the public domain under [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/).

Known third-party sources include CC0-1.0 code and the SplitMix64 finalizer. SplitMix64 is provided under its author's public-domain dedication and unconditional permission, a declaration distinct from CC0-1.0; see its [original notice](LICENSES/LicenseRef-SplitMix64-Public-Domain.txt).

The original public domain dedications and license terms remain applicable to those third-party works. I do not claim ownership of or purport to waive any rights held by third parties.

Bundled ACL code used by verification and examples is covered by [ACL's CC0 1.0 dedication](Verify/current/vendor/ACL_LICENSE).

Third-party source URLs are recorded in the corresponding source files, including for CC0-1.0 and public-domain code.

All code is provided without warranty, to the extent permitted by applicable law.
