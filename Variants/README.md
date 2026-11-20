# 安全版ライブラリ

入力検査は既定で有効です。貼り付け前に `#define SKIP_RUNTIME_CHECKS` を書くと省略できます。

- [コンパイラ共通](Portable)
- [GCC専用](GCC)
- [Clang専用](Clang)
- [MSVC専用](MSVC)

配置と公開APIは原本と共通です。各版はCPU判定と標準演算への切り替えを保持します。生成方法は[配布版の説明](../Document/fast_variants.md)、速度優先版は[Fast](../Fast/README.md)を参照してください。
