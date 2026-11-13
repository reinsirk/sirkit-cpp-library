# 直線集合2Dの2周目確認（2026-10-30）

`DataStructure/line-container-2d.cpp`をP-Coreで再検討した。**追加採用なし。本体・公開API・main・共通入出力・同梱1D本体は維持した。** 独立したAPI/所有権試験と確保失敗試験を追加した。

[412409](https://judge.yosupo.jp/submission/412409)のAC・274msは旧641413版の記録。現行cdb版とのソース一致は得ておらず、現在版のオンラインACに書き換えていない。以前からの現行版再提出待ちは保持する。

## 比較した案

| 案 | 利得 | 採用しなかった理由 |
|---|---|---|
| 各包絡にstd::node_typeを1枠保持し削除ノードを再利用 | 放物線2万追加の割当40000→20003回 | 大規模構築約20〜23%増、公式放物線も約2〜3%増 |
| 唯一の線・中央区間で整数比の除算を省く | 単一点の照会約6〜7%短縮 | 他のAPI・公式ケースで退行。歴史的な不採用案のnative再確認として記録 |
| 追加処理の強制inlineを外す/outlineする | 追加のみの条件で約4%短縮 | half等で約5%増 |
| 係数2個を標準memcpyでコピーし自動SIMDコピーを促す | 一部の放物線/halfで短縮 | 他の構築・整数照会・浮動APIで退行 |
| 自動inline＋バイトコピーの組合せ | 最終GCC通常で追加のみ約7%、half約5%短縮 | GCC nativeのparabola:minline約20%増、通常GCCの軸照会約12%増、Clangの公式ケース約9〜12%増など |

確保回数や命令数の削減だけで採用せず、main全体と公開APIの両方で判断した。手動のISA命令やCPU判定は追加していない。バイトコピー候補にはSIRKIT_DISABLE_SIMDで旧コピー・inline指定へ戻る経路を作り確認したが、候補自体を不採用にした。

## 測定と正しさ

CPUはIntel i9-14900KのP-Core、GCC15.2/Clang21・C++23・O2・検査省略。GCC native、通常GCC/Clang、SIMD無効のGCC/Clangを分けた。最終は他の重い作業を止め、CPU12でbaseline/control/candidateを順序を変えて6回実行。同じbaselineのcontrolを残し、小さな揺らぎを利得とは扱わない。全15公式入力と期待出力は、公式生成器・別方式の公式Li Chao解と上流hash.jsonで照合した。合計6750計時項目、1620プロセスで出力が一致した。

独立試験は全登録点の内積を参照に、値・最適点・optional・丸め・範囲制限・コピー・ムーブ・swap・巨大な積の相殺・比のoverflowを確認する。確保失敗時の全消去、コピー失敗時の保持、move/swap無確保、スコープ終了時の解放も確認した。baseline/reuse×GCC/Clang×checked/skip×2試験の16構成が成功。node再利用試作のASan/UBSan2構成も成功したが、不採用の試作と現行本体の記録は区別した（LSan無効）。

係数コピー候補もGCC/Clang・検査有効/無効・SIMD有効/無効の24構成で独立照合に成功した。正しさが理由でなく、公開API全体で速度を維持できなかったため採用していない。変更していない本体に、同じ回帰試験を再度すべて走らせることはしない。

## 提出と資料

現行版の提出用コードは[line_container_2d_min.cpp](../Verify/current/submissions/line_container_2d_min.cpp)。mainは点の追加とmin_rounded_ll(x,1)の呼び出しだけで、バッチ化・入力分布による切り替えは入れていない。提出コメントは最新の既知URL412409だけを保持する。

[独立API試験](../Verify/current/tests/line_container_2d_reuse_test.cpp) / [確保失敗試験](../Verify/current/tests/line_container_2d_allocation_test.cpp) / [全データ・再現コード](performance-line-container-2d-round2-2026-10-30.json) / [検証記録](verify-line-container-2d-round2-2026-10-30.json)
