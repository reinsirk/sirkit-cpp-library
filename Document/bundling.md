# 選択したライブラリを一つにまとめる

`Scripts/bundle_libraries.py`は指定したライブラリを指定順に連結し、同一の共有部品の重複を省きます。main・入出力・外部ヘッダーは追加しません。ScriptsとVariants/manifest.jsonを含む完全版リポジトリで実行します。

```sh
python3 Scripts/bundle_libraries.py --profile normal \
  --library Math/NTT.cpp --library Math/Formal_Power_Series.cpp \
  --output submission_libraries.cpp --report bundle_report.json
```

## オプション

| 指定 | 動作 |
| --- | --- |
| `--profile` | normal・gcc・clang・fast-gcc・fast-clangから選ぶ。Fastの前提は[配布版の説明](fast_variants.md)を参照。 |
| `--library` | 通常版の正規パスを繰り返し指定する。temp.cpp・temp2.cppは対象外。 |
| `--output` / `--report` | 結合先 / 変更前後のバイト数・SHA-256・省略部品を記録するJSON。 |
| `--no-deduplicate` | 重複省略を行わず結合する。 |
| `--allow-oversize` | 既定の512 KiB上限を解除する。 |

## 出力と制約

現行manifestとソースのSHA-256を照合し、先頭にライセンス・出典・API説明、その後にC++本文を並べます。標準ヘッダーと条件分岐は残します。#lineで元の位置を示すため、診断や__FILE__・__LINE__は元ソースに対応します。Mintなどの利用側の型は結合テキストより前に定義してください。

同じガード・同じ本文のSIRKIT_*_DEFINED部品で、先行部品が確実に定義済みとなる場合だけ重複を省きます。未知の条件・別本文・外側のelseなどがある部品は保持します。undefやmacroのpush/pop・動的pragma・token pasteも考慮し、安全に判断できない省略を無効にします。

独自ファイルやmacroによるinclude、既存の#line、digraphディレクティブ、raw stringと物理行継続を併用するソースは扱いません。標準・明示されたコンパイラ用ヘッダー以外の依存は拒否し、ACLヘッダーを展開する機能もありません。C++23の空白を挟む行継続は考慮します。

512 KiBを超える場合は終了コード2で、既存の出力を保持します。manifestのソースやテンプレートへの上書きも拒否します。結合後は使うコンパイラ設定でコンパイルしてください。詳細な構文判定と検証記録は[Audit](../Audit/)に置きます。
