# 最適化と再提出準備（2026-11-13）

過去にACのあるVerifyコード101件について、Codex→Fable 5.2の検討を完了しました。再提出後に見直したDijkstra・LCA・SCCも再確認済みです。Z Algorithmは別枠の既存完了分です。Library Checkerはmainと入出力を含め、AtCoderは本体と素直なAPI呼び出しを対象にしています。

現在版の再提出待ちは87件、本文照合済みACはZ Algorithmを含む8件です。AtCoder7件は画像AC・実提出本文未照合のまま保持しています。旧ACを変更後のソースへ転記していません。topologicalのURLは80790405へ反映し、利用者から気になる差がないとの回答によりAtCoderをFastest比較から外しました。

## Fastestとの差

Dijkstraは辺の配置・確保、LCAは既存のO(1)照会を保った構築・配置の改善が有効でした。SCCは構築中に得た次数情報で出次数1のグラフを処理し、一般グラフは既存のTarjanへ戻します。3件とも公開APIを保っています。

HLDの経路和はEuler順の根からの和・LCAを使うO(log N)解法と、汎用HLDのO(log² N)との違いがあります。経路合成にも集約構造の違いがあります。独立に測った置換は入力によって回帰したため、既存HLDを維持しました。他者の提出時間は入出力や実装条件を含む値で、本体だけの差とは区別しています。

[比較10件と測定結果](fastest-reconsideration-2026-11-13.md)・[Codex段階の固定記録](fastest-reconsideration-codex-2026-11-13.json)・[FableによるGraph3の追加改善](optimization-fable-batch-graph-followup-2026-11-13.json)

## Fable提案の採否

本体ではLCAのCSR offset保持、SCCの後続頂点・訪問状態の統合、PrimeSieveの間引き列挙API、文字列用SA-ISの記号配列の16bit化を採用しました。SA-ISは整数列・再帰・LCPを保ち、同じ設定の最大50万文字の4種×2問題×2コンパイラで全体約1.7〜4.3%短縮しました。新コードのGCC設定は別に比較し、suffix_arrayはO3,unroll-loops、number_of_substringsはO3を選びました。PrimeSieveは大きい篩のstride列挙で約9〜25%短縮しましたが、小stepの微差も記録し、全入力の一律改善とは説明していません。

Verifyのmainでは、affine値の32bit格納、rollback DFSの不要な末端処理の除去、畳み込み入力配列のmove、OR形式の反転入出力、偏角ソートのAngleLessを採用しました。畳み込み8件は最大乱数で全体約5〜11%短縮し、入力コピー2本分のメモリを削減しました。偏角ソートは変更後にGCC設定を比較し、O3を選びました。

HashSetの事前確保やprobe、Waveletの配置、Superset融合、COM_many、凸包の処理融合、面積・点と線分の距離、LCPのΦ方式、Trieの事前確保、rolling_hash出力変更は、特定入力での回帰や効果不足により不採用です。正しさを通過した案も、速くなった入力だけを根拠に採用していません。

本体・main・入出力・コンパイラ設定の効果と、異なる実験の短縮率は分けて記録しています。共通I/Oは今回の再検討で変更していません。各提案のbefore/after、独立照合、測定の生データ、再現用コードは各batchのJSONとZIPに保存しました。

## 検証・利用枠

最終のPythonテスト276件と、生成器・API配置・Verify・README集計の同期検査15件を通過しました。変更に応じたC++23の検査有効/無効・標準代替・生成版の検証は、各採用案の記録にあります。過去の229件全ビルドと、変更後に実行した検査を区別しています。

11月13日19:40の通常リセット後にFableを再開しました。最後の実行は成功・completed・実モデルclaude-fable-5-2を確認し、5時間枠の最終確認値は96%です。追加利用・無料リセット・別アカウント・別モデルは使っていません。未完了/error応答を完了扱いにしていません。

[最終JSON・再提出用ファイルのSHA](optimization-final-2026-11-13.json)・[件別の現在版と段階](../Verify/optimization/remaining-2026-11-13.json)・[当初101件の記録](remaining-library-optimization-2026-11-13.json)
