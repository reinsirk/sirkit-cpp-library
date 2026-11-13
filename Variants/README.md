# コンパイラ別ライブラリ

- [GCC](GCC)：GCC用の分岐だけを残した通常版。
- [Clang](Clang)：Clang用の分岐だけを残した通常版。

各フォルダ内の配置と公開APIは通常版と同じ。CPUの対応判定や標準演算への切替、入力検査の設定は保持する。

生成方法は[説明](../Document/fast_variants.md)、速度優先版は[Fast](../Fast/README.md)を参照。
