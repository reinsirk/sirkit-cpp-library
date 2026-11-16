# ライブラリページ

[competitive-verifier 4.2.2](https://github.com/competitive-verifier/competitive-verifier/tree/v4.2.2)で、通常版のAPI・使用例・ソース・検証結果をまとめます。`Document/`を説明の原本とし、ページ用の複製は生成時だけ作ります。

共通文書を参照するファイルの名前は[ページタイトル設定](../.competitive-verifier/page-titles.json)で区別します。一覧と各ページの見出しへ同じ名前を使い、重複するVerifyの問題名などには対象ファイルを添えます。

用途別ファイルには[説明文書の設定](../.competitive-verifier/page-documents.json)から`Document/pages/`の個別ガイドを割り当て、他方専用のAPIや使用例を混ぜません。本文は関数ごとの見出し・シグネチャ・短い説明・制約・計算量を先に置き、検証結果は後ろに折り畳みます。「ソースのAPI注記」は対象ソース先頭の説明を直接使い、同梱する型・関数も含めます。

## 表示する結果

| 項目 | 内容 |
| --- | --- |
| オンラインVerify | 現行の提出全文SHAと保存した判定を照合。旧版のACは区別する。 |
| 正常系 | 入力例・独立参照・checked契約テストの照合結果。 |
| 失敗系 | 期待した型の例外と、失敗後にも状態が保たれることを検査する。 |
| Library Checker公式ケース | 外部で生成した全ケースを公式checkerで照合。プログラム全体の時間も記録する。 |
| 本体性能 | 同じ入力・コンパイラ・ハーネスで比較した中央値・生データ・実行環境。 |
| ASan/UBSan・Coverage | checked実行の診断と、通常版本体の行・分岐到達状況。 |

結果がない対象は「未実施」、記録時からソースが変わった対象は「ソース変更後は未検証」です。代表テストの成功を全APIの保証にはしません。失敗系は現在Fenwick Tree・Segment Tree・Lazy Segment Tree・Union Find・Low Link・Rolling Hash、本体性能はFenwick Tree・Union Find・Rolling Hashから開始しています。

## ローカルで確認

Python 3.11以上、Ruby 3.3、Bundler 2.6.9を用意します。

```sh
python3 -m venv /tmp/sirkit-pages-venv
/tmp/sirkit-pages-venv/bin/pip install -r .competitive-verifier/requirements.txt
python3 Scripts/ci_contracts.py --compiler g++ --output /tmp/contracts
python3 Scripts/ci_benchmark.py --base origin/main --output /tmp/performance
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

`main`向けPRで正常系・失敗系・Sanitizer・Coverage・代表性能と、変更対象の公式ケースを実行し、ページ一式を`library-pages-preview`成果物へ保存します。公式ケースは8分割です。CI環境の時間はP-Coreの手元測定やオンライン時間と区別し、速度差だけでは不合格にしません。

mainへ取り込んだPRのcloseイベントで、取り込まれたcommitから公開版を生成します。直前の同じPR headのCI成果物を使い、再度SHAを照合します。成果物が失われた場合は「未実施」とし、過去の成功を補いません。未取り込みPR・通常push・手動起動では公開しません。

初回はGitHubの **Settings → Pages → Build and deployment → Source → GitHub Actions** を選びます。公開先は[GitHub Pages](https://reinsirk.github.io/sirkit-cpp-library/)です。開発中の未コミット変更を`--production`で公開版扱いにはできません。

生成ページへコピーされるcompetitive-verifierのlayout・CSS・JSはMITです。[原文](https://github.com/competitive-verifier/competitive-verifier/blob/d6bfe076aa6ae891f93588d470c7886ad6208bd9/LICENSE)と[第三者表示](https://github.com/competitive-verifier/competitive-verifier/blob/d6bfe076aa6ae891f93588d470c7886ad6208bd9/3rd-party-license.txt)はリポジトリに保存せず、生成時に4.2.2の固定commitから取得してサイトへ同梱します。SHA-256を照合し、取得失敗・内容不一致なら生成を停止します。取得先・commit・SHAはサイトの`licenses/sources.json`に記録します。通常版C++の提供条件とは適用範囲を分けます。既定のMinimalテーマは[CC0-1.0](https://github.com/pages-themes/minimal/blob/master/LICENSE)です。
