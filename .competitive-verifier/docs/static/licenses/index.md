---
layout: default
title: ソースコードとサイトのライセンス
permalink: /licenses/index.html
---

[版を選ぶ]({{ '/' | relative_url }})

# ソースコードとサイトのライセンス

## C++ライブラリ

このリポジトリの独自実装には [CC0 1.0]({{ site.github.repository_url }}/blob/{{ site.branch_name }}/LICENSE) を適用します。外部由来のコードには元の提供条件が適用されます。ダウンロードした `.cpp` を使うときは、ファイル冒頭の `SPDX-License-Identifier`、出典URL、適用範囲を確認してください。

- [ライブラリのライセンス説明]({{ site.github.repository_url }}/blob/{{ site.branch_name }}/Readme.md#license)
- [SplitMix64の提供条件]({{ site.github.repository_url }}/blob/{{ site.branch_name }}/LICENSES/LicenseRef-SplitMix64-Public-Domain.txt)
- [検証と使用例に含まれるAtCoder Libraryの提供条件]({{ site.github.repository_url }}/blob/{{ site.branch_name }}/Verify/current/vendor/ACL_LICENSE)

## サイトの表示に使う部品

competitive-verifier 4.2.2のレイアウト・CSS・JavaScriptにはMITライセンス、JekyllのMinimalテーマにはCC0 1.0が適用されます。

- [competitive-verifierのMITライセンス原文]({{ '/licenses/MIT-competitive-verifier.txt' | relative_url }})
- [competitive-verifierの第三者ライセンス表示]({{ '/licenses/competitive-verifier-third-party-notices.txt' | relative_url }})
- [同梱した文書の取得元とSHA-256]({{ '/licenses/sources.json' | relative_url }})
- [Minimalテーマのライセンス原文](https://github.com/pages-themes/minimal/blob/master/LICENSE)
