# ライブラリページ

[competitive-verifier 4.2.2](https://github.com/competitive-verifier/competitive-verifier/tree/v4.2.2)で、API・使用例・ソース・検証結果をまとめます。`Document/`を説明の原本とし、ページ用の複製は生成時だけ作ります。

共通文書を参照するファイルの名前は[ページタイトル設定](../.competitive-verifier/page-titles.json)で区別します。一覧と各ページの見出しへ同じ名前を使い、重複するVerifyの問題名などには対象ファイルを添えます。

用途別ファイルには[説明文書の設定](../.competitive-verifier/page-documents.json)から`Document/pages/`の個別ガイドを割り当て、他方専用のAPIや使用例を混ぜません。本文は関数ごとの見出し・シグネチャ・短い説明・制約・計算量を先に置き、検証結果は後ろに折り畳みます。「ソースのAPI注記」は対象ソース先頭の説明を直接使い、同梱する型・関数も含めます。

## コードを取得

トップページで安全版または速度優先版を選び、使うコンパイラを選びます。コンパイラ共通・GCC専用・Clang専用・MSVC専用のそれぞれにライブラリ一覧があり、選んだ版のコードを取得できます。各一覧からAPIの説明を開くと、同じ版の一覧へ戻れます。

安全版は入力検査が既定で有効です。利用側で先に `#define SKIP_RUNTIME_CHECKS` を置けば対象の検査を省略できます。速度優先版は対象の検査を生成時に省き、一部はメモリの解放も省きます。残る検査とメモリ管理は[配布版の説明](fast_variants.md)を参照してください。

ダウンロード先は生成サイトに同梱する `.cpp` ファイルです。[生成版の一覧](../Variants/manifest.json)と照合して8構成を掲載し、原本と生成版のSHA-256、ページのリンク先、ダウンロード内容を確認します。APIの説明と検証結果は通常版について表示します。

## 表示する結果

| 項目 | 内容 |
| --- | --- |
| オンラインVerify | 現行の提出全文SHAと保存した判定を照合。旧版のACは区別する。 |
| 正常系 | 入力例・独立参照・checked契約テストの照合結果。 |
| 失敗系 | 期待した型の例外と、失敗後にも状態が保たれることを検査する。 |
| Library Checker公式ケース | 外部で生成した全ケースを公式checkerで照合。プログラム全体の時間も記録する。 |
| 性能退行 | 全103本のVerify全体時間と代表3本の本体時間をPRのベースと比較。入力規模・両版の中央値・判定を表示。 |
| ASan/UBSan・Coverage | checked実行の診断と、通常版本体の行・分岐到達状況。coverageは両方100%を必須にする。 |

結果がない対象は「未実施」、記録時からソースが変わった対象は「ソース変更後は未検証」です。代表テストの成功を全APIの保証にはしません。失敗系は現在Fenwick Tree・Segment Tree・Lazy Segment Tree・Union Find・Low Link・Rolling Hashが対象です。本体だけの補足性能はFenwick Tree・Union Find・Rolling Hashを測ります。

## ローカルで確認

Python 3.11以上、Ruby 3.3、Bundler 2.6.9を用意します。

```sh
python3 -m venv /tmp/sirkit-pages-venv
/tmp/sirkit-pages-venv/bin/pip install -r .competitive-verifier/requirements.txt
python3 Scripts/ci_contracts.py --compiler g++ --output /tmp/contracts
python3 Scripts/ci_performance_all.py --cache /tmp/performance-cache --output /tmp/performance
/tmp/sirkit-pages-venv/bin/python Scripts/build_library_pages.py \
  --report /tmp/contracts --report /tmp/performance \
  --output /tmp/library-pages --baseurl '' --render
python3 -m http.server 8000 --directory /tmp/library-pages/site
```

ブラウザで`http://localhost:8000/`を開きます。`--render`を省くとJekyll用Markdownだけを生成します。通常のGit index・object DBは変更しません。未追跡のライブラリもプレビューへ含め、生成中に入力が変われば再生成を要求します。

## Library Checkerの問題資産

取得する公式版は[`Verify/ci/library_checker.json`](../Verify/ci/library_checker.json)のcommitに固定します。以下は公式Unionfindを使う例です。

```sh
python3 Scripts/ci_library_checker.py --id dynamic_union_find \
  --cache /tmp/library-checker-cache --output /tmp/library-checker-result
```

`--all`でLibrary Checker対象全件、`--base origin/main`で変更に関係する対象を選べます。`--shard-index`・`--shard-count`で分割できます。問題のsource・generator・checker・生成データ・実出力・生ログは外部キャッシュに置き、リポジトリ・ページ・CI成果物には含めません。取得先のライセンス文書は保持します。Apache-2.0等の提供条件をCC0へ置き換える扱いではありません。

ページへ渡すのは、対象ソース・提出全文・公式commit・checker・ケース集合のSHA、件数、判定、実行条件と時間です。構成問題など非一意の出力も公式checkerで判定し、単純な文字列比較で代用しません。

## PRと公開

`main`向けPRで正常系・失敗系・Sanitizer・Coverage・全103本と代表本体の性能退行検査、全LC対象の公式ケースを実行し、ページ一式を`library-pages-preview`成果物へ保存します。公式ケースと全ライブラリ性能は各8分割です。別のMax・構造ケースはActionsの`stress-cases`成果物へ保存します。性能は入力規模・実行環境、PRのベースと変更後の時間、退行判定を表示します。Verify全体・本体のみ・オンラインの測定結果はそれぞれ表示します。

mainへ取り込んだPRのcloseイベントで、取り込まれたcommitから公開版を生成します。直前の同じPR headのCI成果物を使い、再度SHAを照合します。GitHub APIの関連PR一覧が空の場合は、Verifyの実行名に含まれるPR番号とheadのリポジトリ・ブランチ・SHAで照合します。成果物が見つからない場合は「未実施」とし、過去の成功を補いません。生成が成功すると、別の公開ジョブが`main`上で動き、生成元が現在の`main`の先頭と一致する場合に公開します。

初回はGitHubの **Settings → Pages → Build and deployment → Source → GitHub Actions** を選びます。`github-pages`環境の公開許可ブランチは`main`です。未設定のため `Library pages` が失敗した場合は、設定後に同じ実行を再実行します。公開先は[GitHub Pages](https://reinsirk.github.io/sirkit-cpp-library/)です。開発中の未コミット変更を`--production`で公開版扱いにはできません。

生成ページへコピーされるcompetitive-verifierのlayout・CSS・JSはMITです。[原文](https://github.com/competitive-verifier/competitive-verifier/blob/d6bfe076aa6ae891f93588d470c7886ad6208bd9/LICENSE)と[第三者表示](https://github.com/competitive-verifier/competitive-verifier/blob/d6bfe076aa6ae891f93588d470c7886ad6208bd9/3rd-party-license.txt)はリポジトリに保存せず、生成時に4.2.2の固定commitから取得してサイトへ同梱します。SHA-256を照合し、取得失敗・内容不一致なら生成を停止します。取得先・commit・SHAはサイトの`licenses/sources.json`に記録します。通常版C++の提供条件とは適用範囲を分けます。既定のMinimalテーマは[CC0-1.0](https://github.com/pages-themes/minimal/blob/master/LICENSE)です。
