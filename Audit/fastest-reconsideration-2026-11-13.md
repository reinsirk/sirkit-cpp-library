# Fastestとの差の再検討（2026-11-13）

今回再提出されたLibrary Checker 10件を調べました。AtCoderは利用者から気になる差がないとの回答を受け、速度比較の対象から外しました。`topological.cpp`の提出URLは80790405へ更新済みです。

下表は改修前の提出結果です。現在のテストケース集合との一致は確認しましたが、過去の他者提出のコンパイラ版はAPIから確定できません。オンライン時間はプログラム全体の記録であり、本体だけの性能差とは区別します。

| 対象 | 今回提出 / Fastest | 主な差と判断 |
| --- | --- | --- |
| Dijkstra | [141ms](https://judge.yosupo.jp/submission/416737) / [28ms](https://judge.yosupo.jp/submission/415573) | 同じ最短路でも辺の配置・確保が大きい。公式の線状ケースでは終点早期終了の利益はなかった。辺列APIと内部CSRを追加。 |
| LCA | [113ms](https://judge.yosupo.jp/submission/416743) / [24ms](https://judge.yosupo.jp/submission/278239) | 既存getもO(1)。親配列に特化した構築・配置・入出力との差。全機能を残して辺列APIを追加。 |
| Jump on Tree | [200ms](https://judge.yosupo.jp/submission/416738) / [56ms](https://judge.yosupo.jp/submission/403437) | 小ブロックに詰めた経路索引、構築、先読み、入力処理の差。単なるSIMDの有無ではない。今回は既存HLDを維持。 |
| Path Sum（所有型） | [251ms](https://judge.yosupo.jp/submission/416740) / [100ms](https://judge.yosupo.jp/submission/278241) | Euler順の部分木更新と根からの和・LCAを使う解法はO(log N)、汎用HLDはO(log² N)。独立に試した置換は一律には速くならなかった。 |
| Path Sum（callback） | [281ms](https://judge.yosupo.jp/submission/416741) / [100ms](https://judge.yosupo.jp/submission/278241) | 同上。公開rangeの意味を変える置換はしない。 |
| Path Composite | [241ms](https://judge.yosupo.jp/submission/416739) / [116ms](https://judge.yosupo.jp/submission/319096) | 木の集約構造が異なる。公式HLD最悪ケースでも差を再現したが、線状入力では現行が速い例もある。 |
| SCC | [49ms](https://judge.yosupo.jp/submission/416748) / [9ms](https://judge.yosupo.jp/submission/413708) | 次数に応じた処理・配置・入出力の差。出次数1の汎用経路分解を追加し、一般グラフは既存Tarjanへ戻す。 |
| 点二重連結成分 | [140ms](https://judge.yosupo.jp/submission/416745) / [68ms](https://judge.yosupo.jp/submission/287412) | 森の直接出力、ブロックごとの確保・出力配置が大きい。独立な森の短絡案は一般乱数で少し回帰し不採用。 |
| 二重辺連結成分 | [31ms](https://judge.yosupo.jp/submission/416746) / [25ms](https://judge.yosupo.jp/submission/412935) | 取得時11位。reserve追加も一貫改善せず現行維持。 |
| TwoSAT | [80ms](https://judge.yosupo.jp/submission/416749) / [38ms](https://judge.yosupo.jp/submission/405839) | 取得時3位。上限付き伝播とSCCの併用は乱数で速いが、巻き戻しを誘う鎖で回帰し不採用。 |

採用した変更のローカル測定は次の通りです。GCC15.2/Clang21、同じ入力・入出力・検査設定で交互に測定し、各実験内の中央値を比較しました。辺列APIの構築とmainを含む全体比較です。異なる実験の短縮率を加算したり、オンライン時間へそのまま当てはめたりはしません。

| 変更 | 同条件で確認した効果 | 保持した契約 |
| --- | --- | --- |
| Dijkstra辺列版 | 公式line: GCC 201.72→115.74ms、Clang 209.84→111.17ms。比較10入力すべて短縮。内部重み32bit化も配置の効果に含む。 | WGraph版、全頂点の距離、経路復元。辺列版は構築中に全端点・重みを検査。 |
| LCA辺列版 | 公式4入力: GCC約18〜33%、Clang約14〜25%短縮。peak RSSは約15MiB減。 | 森・任意の根・祖先・部分木・パス・コピー・再構築を含む既存機能。 |
| SCC出次数1 | 公式large_cycle: GCC 27.55→16.37ms、Clang 26.39→15.29ms。一般DAG・乱数・密グラフはほぼ同等。 | 成分IDのトポロジカル順、群内昇順、例外時の旧結果保持。最大時間を決めるケースが移るため、全体4割短縮とは主張しない。 |

共通入力も独立な4候補を測りました。全入力保持は平均で約1.4〜8.8%遅く、mmapは平均約1.9%短縮する一方でLCA約4.6%・TwoSAT約8.3%の回帰と最大約17MiBのRSS増があり、一律採用を見送りました。既存共通I/OのSHAは維持しています。

GCC設定は新しい実装でも比較し直し、Dijkstra/LCAはO3、SCCはO3,unroll-loopsを維持しました。3件の通常・派生版は計76ビルド構成・96実行を通過しました。別途、独立参照、検査有効/無効、標準代替、確保失敗、所有・コピー・再構築も確認しています。

Fableでも変更後の3件を確認し、LCAではCSRの区間情報をoffset配列だけで持つ変更、SCCでは後続頂点と訪問状態を同じ整数欄へ保持する変更を追加採用しました。LCAはCodex版との別測定でGCC約4〜18%・Clang約6〜9%短縮し、RSSは約3.8MiB減りました。SCCは公式cycleで追加約5.4%・8.9%短縮しました。Dijkstraのoffset32bit化は一貫して速くならず不採用です。上表のCodex測定と追加測定の短縮率は加算しません。

3件ともFable完了として再提出を待つ状態です。変更前のACを現在版へ引き継いでいません。他者の原文や改変した比較プログラムは再配布していません。

[Fableの確認・追加測定](optimization-fable-batch-graph-followup-2026-11-13.json)・[固定したCodexのJSON記録](fastest-reconsideration-codex-2026-11-13.json)・[改修コードと検証資料](data/fastest-reconsideration-codex-2026-11-13.zip)・[現在の段階と提出SHA](../Verify/optimization/remaining-2026-11-13.json)
