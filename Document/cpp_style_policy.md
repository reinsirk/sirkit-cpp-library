# C++の作法と適用方針

通常版へCore GuidelinesとGoogle Styleの自動検査を適用します。公開APIは`sirkit`内に置き、型・操作の呼び出し形式は現行方針を維持します。2つのCIプロファイルで結果を分け、ソースを複製しません。速度優先版は作法検査の対象外です。

| 資料 | 主な目的 |
| --- | --- |
| [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) | 所有権・寿命・安全なAPI・型の使い方。 |
| [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) | Googleの開発環境に合わせた設計・言語機能・命名・書式の統一。 |

両ガイドは完全には一致しません。例えばCoreのNL.10はunderscore形式を好み、Googleの型・通常の関数名はPascalCaseです。全面準拠を意味する単一の設定にはせず、[CIで検査できる項目](ci_quality.md)を明示します。

## 決定済み

- 標準C++23まで使用可。Googleの現在のC++20指定には制限しない。
- メイン版では`__int128`・SIMD等の拡張を許容し、標準C++の代替経路を維持する。
- 作法検査に使うガイドはCore GuidelinesとGoogle Styleの2つに絞る。
- 公開APIを`namespace sirkit`へ、内部実装を`sirkit::detail`へ配置する。ライブラリ内でusing-directiveは使わず、競プロ用mainでは使用可能とする。
- 無限大を表す共有定数は`kInf`へ統一する。定数は`k`＋PascalCaseを基本とし、変更可能な変数に`k`は付けない。

## 現行方針と衝突する項目

以下は未採用で、利用者の判断待ちです。この表の形式規則を理由に、判断前に公開APIや動作を変更しません。

| 項目 | ガイドの規則 | 現行方針と影響 |
| --- | --- | --- |
| APIの型・操作名 | Googleは型・通常の関数をPascalCaseにする。 | `segtree`等のACL互換名と衝突。改名はAPI変更になる。定数の`k`接頭辞は採用済み。 |
| 配布単位・ガード | Googleは`.h`とファイル全体のパス由来ガードを基本とする。 | 現在は貼り付け用`.cpp`で、機能別の`SIRKIT_`ガードと共有部品ガードを使う。配布・連結の設計に影響。 |
| 例外 | GoogleはC++例外を使わない。 | 現在は不正引数・範囲外・オーバーフロー等を例外で報告。変更にはエラー処理の再設計が必要。 |
| 浮動小数点 | Googleは`long double`を使わない。 | 幾何等の公開型と計算に使用。`double`への単純置換は精度・判定結果に影響する。 |
| 拡張の配置 | Googleは拡張を使う移植性ラッパーを共通ヘッダーへ集約する条件を設ける。 | メイン版では拡張を許容するが、各ファイルを単独で貼り付けられる方針とは配置上の差がある。 |

根拠: Googleの[名前空間](https://google.github.io/styleguide/cppguide.html#Namespaces)・[命名](https://google.github.io/styleguide/cppguide.html#Naming)・[ガード](https://google.github.io/styleguide/cppguide.html#The__define_Guard)・[例外](https://google.github.io/styleguide/cppguide.html#Exceptions)・[浮動小数点](https://google.github.io/styleguide/cppguide.html#Floating-Point_Types)・[拡張](https://google.github.io/styleguide/cppguide.html#Nonstandard_Extensions)、Coreの[NL.9](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rl-all-caps)・[NL.10](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rl-underscore)。

## ほかの代表的な資料

[LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html)はLLVMプロジェクトの規約、[SEI CERT C++ Coding Standard](https://cmu-sei.github.io/secure-coding-standards/sei-cert-cpp-coding-standard/)は安全性上の欠陥を防ぐ規則、[C++ Best Practices](https://github.com/cpp-best-practices/cppbestpractices)は設計・ツール等の実践集です。今回はCIの検査対象には追加しません。
