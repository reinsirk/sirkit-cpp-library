# ライブラリページ

[competitive-verifier 4.2.2](https://github.com/competitive-verifier/competitive-verifier/tree/v4.2.2)を使い、API・使用例・ソース・検証結果を掲載します。説明の原本は`Document/`に置き、ページ用の複製は生成時だけ作ります。

[ページタイトル設定](../.competitive-verifier/page-titles.json)で、同じ文書を参照するファイルにも別々のページ名を付けます。一覧と各ページの見出しには同じページ名を使い、Verifyの問題名などが重複する場合は対象ファイルを添えます。

用途別ファイルには[説明文書の設定](../.competitive-verifier/page-documents.json)から`Document/pages/`の個別ガイドを割り当て、他方専用のAPIや使用例を混ぜません。本文は関数ごとの見出し・シグネチャ・短い説明・制約・計算量を先に置き、検証結果は後ろに折り畳みます。「ソースのAPI注記」は対象ソース先頭の説明を直接使い、同梱する型・関数も含めます。

## コードを取得

トップページで通常版か速度優先版を選び、次に使うコンパイラを選びます。コンパイラ共通・GCC専用・Clang専用・MSVC専用の一覧から、選んだ版のコードを取得できます。一覧からAPIの説明を開いたあとも、同じ版の一覧に戻れます。

通常版は入力検査が既定で有効です。利用側でコードを貼り付ける前に `#define SKIP_RUNTIME_CHECKS` と書くと、対象の検査を省けます。速度優先版は競技プログラミングの提出向けです。生成時に対象の検査を省き、一部の実装ではメモリの解放も省きます。残る検査とメモリ管理は[配布版の説明](fast_variants.md)を参照してください。

ダウンロード先は生成サイトに同梱する `.cpp` ファイルです。[生成版の一覧](../Variants/manifest.json)と照合して8構成を掲載し、原本と生成版のSHA-256、ページのリンク先、ダウンロード内容を確認します。APIの説明と検証結果は通常版について表示します。

各ライブラリには必要な自作部品を埋め込んでいるため、表示中のコードやダウンロードした `.cpp` をそのまま貼り付けられます。標準・処理系のヘッダーは `#include` で読み込みます。Mintを引数に取るライブラリでは、Mintを利用側で用意します。複数のライブラリをまとめるときは[結合スクリプト](bundling.md)を使えます。

## 表示する結果

ライブラリ一覧のチェックマークは通常版の検証結果を示します。現行コードとACの提出全文が一致するVerifyが1件以上あれば「現行ACあり」と表示します。ローカルVerifyの通過を示す場合は「通常版のローカルVerify通過」と表示します。

「Verifyプログラムと検証結果」から、各プログラム、提出先の問題、記録済みの提出結果を開けます。現行ACの各行にもチェックマークを付けます。

トップページのバッジから、ライセンスの内訳、最新リリース、main向けPRのVerify実行履歴を開けます。

Last Commitは、トップページではサイトの生成元コミット、ライブラリのページではそのソースを最後に変更したコミットを示します。バッジを選ぶとGitHubで該当するコミットを開けます。

| 項目 | 内容 |
| --- | --- |
| オンラインVerify | 現行の提出全文SHAと保存した判定を照合。旧版のACは区別する。 |
| 正常系 | 入力例・独立参照・checked契約テストの照合結果。 |
| 失敗系 | 契約テストと補足テストで、期待した例外や失敗時の戻り値を照合する。 |
| Library Checker公式ケース | 外部で生成した全ケースを公式checkerで照合。プログラム全体の時間も記録する。 |
| 手元の性能測定（任意） | ローカルプレビューにレポートを渡すと、入力規模・実行環境・中央値などを表示する。 |
| ASan/UBSan・Coverage | checked実行の診断と、通常版本体の行・分岐到達状況。coverageは両方100%を必須にする。 |

失敗系とLibrary Checkerで実行結果が届いていない項目は「未測定」です。Library Checkerに対応するVerifyの登録がなければ「無し」と表示します。記録時からソースが変わった項目は「ソース変更後は未検証」です。失敗系の対象は[補足テストと品質集計の説明](ci_quality.md)を参照してください。

## ローカルで確認

Python 3.11以上、Ruby 3.3、Bundler 2.6.9を用意します。

```sh
python3 -m venv /tmp/sirkit-pages-venv
/tmp/sirkit-pages-venv/bin/pip install -r .competitive-verifier/requirements.txt
python3 Scripts/ci_contracts.py --compiler g++ --output /tmp/contracts
/tmp/sirkit-pages-venv/bin/python Scripts/build_library_pages.py \
  --report /tmp/contracts \
  --output /tmp/library-pages --baseurl '' --render
python3 -m http.server 8000 --directory /tmp/library-pages/site
```

ブラウザで`http://localhost:8000/`を開きます。`--render`を省くとJekyll用Markdownだけを生成します。通常のGit index・object DBは変更しません。未追跡のライブラリもプレビューへ含め、生成中に入力が変われば再生成を要求します。

手元で測った性能をプレビューに載せる場合は、性能レポートの保存先を `--report` で追加します。[手元でライブラリの性能を測る](performance_measurement.md)

## Library Checkerの問題資産

取得する公式版は[`Verify/ci/library_checker.json`](../Verify/ci/library_checker.json)のcommitに固定します。以下は公式Unionfindを使う例です。

```sh
python3 Scripts/ci_library_checker.py --id dynamic_union_find \
  --cache /tmp/library-checker-cache --output /tmp/library-checker-result
```

`--all`でLibrary Checker対象全件、`--base origin/main`で変更に関係する対象を選べます。`--shard-index`・`--shard-count`で分割できます。問題のsource・generator・checker・生成データ・実出力・生ログは外部キャッシュに置き、リポジトリ・ページ・CI成果物には含めません。取得先のライセンス文書は保持します。Apache-2.0等の提供条件をCC0へ置き換える扱いではありません。

ページへ渡すのは、対象ソース・提出全文・公式commit・checker・ケース集合のSHA、件数、判定、実行条件と時間です。構成問題など非一意の出力も公式checkerで判定し、単純な文字列比較で代用しません。

## PRと公開

`main`向けPRでは、正常系と失敗系のテスト、Sanitizer、カバレッジ、Library Checkerの全対象の公式ケースを実行します。公式ケースは8分割で照合し、1ケース30秒で打ち切ります。最大規模・構造別のケースは1ケース60秒で打ち切り、結果を`stress-cases`に保存します。ページ一式は`library-pages-preview`に保存します。

PRを`main`に取り込むと、closeイベントで取り込み後のコミットから公開版を生成します。同じPRの同じheadで直前に実行したCIの成果物を使い、ソースのSHAを改めて照合します。GitHub APIの関連PR一覧が空の場合は、Verifyの実行名に含まれるPR番号とheadのリポジトリ・ブランチ・SHAで照合します。成果物が見つからない場合は「未実施」とし、過去の成功で補いません。生成に成功すると別の公開ジョブが`main`上で動き、ページの生成元がその時点の`main`の先頭と一致する場合に公開します。

初回はGitHubの **Settings → Pages → Build and deployment → Source → GitHub Actions** を選びます。`github-pages`環境の公開許可ブランチは`main`です。未設定のため `Library pages` が失敗した場合は、設定後に同じ実行を再実行します。公開先は[GitHub Pages](https://reinsirk.github.io/sirkit-cpp-library/)です。開発中の未コミット変更を`--production`で公開版扱いにはできません。

サイトの「ソースコードとサイトのライセンス」に、C++ライブラリの提供条件とページの表示に使う部品のライセンスを掲載します。competitive-verifierのレイアウト・CSS・JavaScriptはMIT、既定のMinimalテーマは[CC0-1.0](https://github.com/pages-themes/minimal/blob/master/LICENSE)です。

competitive-verifierの[ライセンス原文](https://github.com/competitive-verifier/competitive-verifier/blob/d6bfe076aa6ae891f93588d470c7886ad6208bd9/LICENSE)と[第三者ライセンス表示](https://github.com/competitive-verifier/competitive-verifier/blob/d6bfe076aa6ae891f93588d470c7886ad6208bd9/3rd-party-license.txt)は、生成時に4.2.2の固定コミットから取得し、サイトに同梱します。リポジトリには保存しません。取得に失敗した場合やSHA-256が一致しない場合は生成を停止します。取得先・コミット・SHA-256はサイトの`licenses/sources.json`に記録します。
