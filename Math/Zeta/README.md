# ゼータ／メビウス変換の選び方

各 `.cpp` は標準ヘッダー以外の依存がなく、そのまま単独で貼り付けられます。
複数の版を併用したり同じコードを貼り付けたりしても、部品ごとのガードで再定義を防ぎます。
全機能が必要なら従来の `../Zeta_Mebius.cpp` を使ってください。API名・処理本体は共通です。

| ファイル | 含まれるAPI | 使用例 |
| --- | --- | --- |
| `Set_Transform.cpp` | `superset/subset_zeta_transform`、`superset/subset_mobius_transform` | `../../Examples/zeta_set_example.cpp` |
| `Bitwise_Convolution.cpp` | AND/OR畳み込み・自己畳み込み、集合4変換 | `../../Examples/zeta_bitwise_example.cpp` |
| `Sieve.cpp` | `IntegerSieve`の最小素因数表、素因数分解、約数列挙、各種数論表 | `../../Examples/zeta_sieve_example.cpp` |
| `Divisor_Multiple_Transform.cpp` | `multiple_*_transform`、`divisor_*_transform`、`IntegerSieve` | `../../Examples/zeta_integer_example.cpp` |
| `Gcd_Lcm_Convolution.cpp` | `gcd_convolution`、`lcm_convolution`、`gcd_square`、`lcm_square`、`IntegerSieve` | `../../Examples/zeta_convolution_example.cpp` |
| `Sparse_Transform.cpp` | `sparse_multiple_zeta_transform`、`sparse_multiple_mobius_transform`、`IntegerSieve` | `../../Examples/zeta_sparse_example.cpp` |

GCD/LCM版に含まれる整数変換は内部処理用です。公開APIの4つの整数変換も直接呼び出す場合は `Divisor_Multiple_Transform.cpp` を併用してください。
整数変換・畳み込みの `IntegerSieve` を省くオーバーロードは、必要な素数列を内部で用意します。
方式と前提は[アルゴリズムの説明](../../Document/zeta_mobius.md)、引数・戻り値は各ファイルのAPI一覧を参照してください。
`SKIP_RUNTIME_CHECKS` を定義すると入力検査を除きます。添字・容量の安全対策は残ります。

## 編集と生成

`components/` が共通部品の編集元です。全機能版と用途別6ファイルを直接編集せず、次で同期します。

```sh
python3 Scripts/generate_zeta.py
python3 Scripts/generate_zeta.py --check
```

生成器は必要な部品を生成ファイルに展開します。利用時にcomponentsや生成器は不要です。
用途別版も全機能版と同じCC0-1.0です。各部品に対応する出典・参考URLは生成物にも残します。
