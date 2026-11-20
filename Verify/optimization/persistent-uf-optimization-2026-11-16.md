# Persistent Unionfindの速度差調査・最適化（2026-11-16）

主因はアルゴリズムの違いと判断した。[Fastest 409831](https://judge.yosupo.jp/submission/409831)は全操作を先読みし、不要な版を剪定した版木をrollback UFで巡回する。現行版は永続配列とサイズによる結合で、先読みなしに任意の保存版を照会・更新する完全永続UF。
前者の保守的な上界はO(N+Qlog(N+1))時間・O(N+Q)領域、後者の操作はO(log²(N+1))、成功する結合ごとにO(log(N+1))ノードを保存する。Nは頂点数、Qは操作数。

公開記録では[変更前417442](https://judge.yosupo.jp/submission/417442)が91ms・113528832bytes、Fastestが15ms・6852608bytes。公式max_random各ケースでも同様の差がある。異なるmain・入出力・コンパイラ設定を含むため、6.07倍を本体実装だけの差とは扱わない。
Verifyをrollbackへ置き換えると完全永続UFの検証にならないため、完全永続性と公開APIを維持した。

## 採用した変更

- UFの親番号は同じ版で明示更新済みなので、初回の添字・版検査後は祖先の配列経路のnull判定を省く。
- `same/issame`は代表まで上らず、探索中に同じ頂点へ到達した時点でtrueを返す。
- main・共通入出力・GCC設定・分岐幅・所有権は変更していない。`Version=Node*`、公開`arr`とNodeの構築形式も維持。

Codexの検討・測定後にFable（`claude-fable-5-2`）が両提出ソースを静的レビューし、変更は妥当、追加提案なしと判定した。Fableはテストを実行していない。

## 測定

GCC15.2、C++23、外側O2でビルドし、CPU27に固定して実行した。LCの埋め込みO3/unroll設定は比較前後で同じ。無作為に交互実行した9回の中央値を使った。入力・main・入出力を固定し、同じ変更前バイナリを別名でも実行して測定値のばらつきを確認した。

| 処理 | 変更前 | 変更後 | 同一バイナリ対照 |
|---|---:|---:|---:|
| LC公式 max_random_00・全体 | 87.209ms | 87.096ms | 87.906ms |
| LC公式 conchon_filliatre_tle_00・全体 | 59.924ms | 59.952ms | 60.235ms |
| 2百万回の過去版same照会・本体のみ | 140.036ms | 125.014ms | 143.383ms |

照会主体の本体測定は約10.7%改善したが、公式LCの全体時間はほぼ同等。15msとの差は埋まっていない。max_randomの最大RSSは前後とも113404KiB。ローカル計測をオンライン提出時間とはみなさない。

2つの公式入力はlibrary-checker-problemsのcommit `73876f1cbcf082d2b22722924b924d3d65a2f00e`に固定し、公式validatorと独立参照解correct_2で検査した。入力と正解出力のSHA-256は、いずれも公式hash.jsonの値と一致した。

## 不採用とした案

| 案 | 判断 |
|---|---|
| 分岐幅2/4/8/32への変更 | 結合主体で速くても、照会主体やvector構築で退行。公開型・既定値も維持する。 |
| 子ポインタを32bit番号へ圧縮 | 結合主体で約20〜35%短縮するが、照会主体で約30〜40%低下。 |
| 内部ノードと葉のpool分離 | 小規模構築が遅く、公開Nodeコンストラクタとの互換検査も不成立。 |
| 確保ブロック拡大・葉の使用部分だけコピー | 安定した全体改善なし。 |
| 未初期化の版配列・char出力・GCC設定 | 一貫した改善なし。 |
| 完全永続UFを維持したオフライン不要版剪定 | 分岐主体では有効だが、公式ケースでは同等〜退行。 |

不採用の試作は一時フォルダ内だけで、リポジトリ本体には反映していない。

## 検証と再提出結果

- GCC/Clang×検査あり/なしの参照テスト20構成が成功。版の分岐、groups、コピー・移動、所有権、汎用値型、確保失敗を含む。
- LC公式2入力とyukicoderの公式3例・独立参照84例・大規模4例を、GCC/Clang×検査あり/なしで全照合。
- 生成版14構成の単独照合と、生成済み提出コード・使用例28条件が成功。
- 通常284本・派生1030本を同期。変更前のACは変更後へ引き継がず、再提出本文を照合して判定を記録した。

再提出された通常版ソースは2本とも現行本文一致AC：

| ソース | 提出先 | 結果 |
|---|---|---|
| [persistent_union_find.cpp](../current/submissions/persistent_union_find.cpp) | [Library Checker Persistent Unionfind](https://judge.yosupo.jp/problem/persistent_unionfind) | [AC・96ms](https://judge.yosupo.jp/submission/417443) |
| [persistent_union_find_union_path_query.cpp](../current/submissions/persistent_union_find_union_path_query.cpp) | [yukicoder No.2295](https://yukicoder.me/problems/no/2295) | [AC・1212ms](https://yukicoder.me/submissions/1199606) |

[Codex記録](../../Audit/Performance/persistent-uf-optimization-codex-2026-11-16.json)・[Fable記録](../../Audit/Performance/persistent-uf-optimization-fable-2026-11-16.json)・[最終記録](../../Audit/Performance/persistent-uf-optimization-final-2026-11-16.json)にソースSHA・実測値・採否を保存。
Fableの最初の広い依頼は出力上限と600秒の時間切れで回答を得られず、完了したレビューとは扱っていない。対象を絞った再実行で2本のレビューを取得した。最終利用率は5時間枠50%、週間Fable69%（全モデル35%）。追加課金・リセット・別アカウントへの変更なし。コミットなし。

## オンライン再提出の確認

LCは14ケースすべてAC。最大値は直前91msから96msとなったが、max_random_04だけ89→96ms、他は同等または1〜3ms短縮だった。単発の公開提出から本体性能の退行とは断定しない。最新Fastestは引き続き15msで、差81ms・6.4倍。yukicoderはsample3・other53の全56ケースAC、1212ms・225536KB。いずれも生成冒頭の提出注記・改行形式以外は現行本文と一致。

[再提出確認記録](../../Audit/Submissions/persistent-uf-optimized-submissions-2026-11-16.json)に取得応答・提出本文のSHAと照合方法を保存。現在の2本は再提出待ちを解除済み。コミットなし。
