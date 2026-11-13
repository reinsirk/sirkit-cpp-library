# 完全永続Union Findの2周目改善（2026-10-30）

**GCCの`groups()`だけを最適化した。** 最初は従来の疎配列参照を使い、親へ進むときだけ確保済みの経路を参照する。`unite`が親のサイズと子の親番号を同じ版に記録するため、この性質は過去版・分岐版でも成立する。頂点番号の大小は使わない。公開API・所有管理・main・共通入出力を保った。

## 採用した範囲

Intel i9-14900KのP-Core12、C++23・O2・検査省略。GCC15.2/15.3の通常/native、Clang21を分け、同一baselineのcontrolと候補を順序を変えて7回測定した。比率は対の中央値。13万頂点の平衡UFの成分列挙は次の通り。100%未満が短縮。

| 条件 | 通常の別プロセス比較 | 同一path/inodeでの比較 |
|---|---:|---:|
| GCC15.2 | 77.2% | 78.2% |
| GCC15.2 native | 61.2% | 61.2% |
| GCC15.3 | 69.2% | 69.5% |
| GCC15.3 native | 71.6% | 72.3% |

大規模平衡条件は約23〜39%短縮。4096頂点の平衡条件も短縮し、疎な分岐版は概ね同程度。別プロセスの空版で約7〜14%の差が出たため、同一path/inodeと、同じUF・allocator上で旧groups本体を複製する補足も行った。後者のGCC空版は概ね±2%、平衡は約14〜40%短縮。複製メソッドは呼び出し文脈も違うため、補足と区別する。Clangでは出荷ヘッダーの実行コードが不変なのに計時差が出る条件もあり、観測差を全て実装効果と扱わない。

**既存Verify mainと`groups`を使わないAPI比較プログラムは、5設定×2種類の全10構成で`.text`が同一。** 結合・通常照会・永続配列・非自明型のコードとデータ構造を変えていない。Clangのgroupsを含む比較プログラムも`.text`が同一。Library Checkerの既存mainはgroupsを呼ばないため、この変更による提出時間の短縮は見込まない。

## 見送った案

| 案 | 利得 | 見送った理由 |
|---|---|---|
| trivial Nodeをbyteブロックとcursorで格納、拡張だけcold helper | 整数配列20万要素の構築約58〜72%、大規模版分岐構築約12〜24%短縮 | `get_two`や疎な版の一部参照が約6〜13%増 |
| blockレコードを24bytes、Storageを40bytesへ戻すpool | 85回の確保サイズ・順序と保存版Nodeの配置まで旧版に一致 | 一部疎版参照は約16〜20%増、配列構築の利得も約9〜12%へ縮小 |
| 深さ4のget/get_two展開 | 一部照会約8〜20%短縮 | 平衡UF約6〜7%増、小規模異要素参照約12〜40%増 |
| 公開find/sizeにも親の確保を使う | 平衡の代表探索・サイズ取得を短縮 | 疎な版の一部照会に増加が残ったため、groupsだけへ限定 |

確保方式の試作は[byte配列の整列規則](https://eel.is/c++draft/expr.new)、[格納域とオブジェクト](https://eel.is/c++draft/intro.object)、[`construct_at`](https://eel.is/c++draft/specialized.construct)も確認した。独立参照・過剰整列・コピー専用型・値操作/確保失敗・所有・ASan/UBSanは成功したが、速度のため採用しなかった。現在のソースにbyte poolは入れていない。

## 検証と提出

採用版は独立24構成に成功。GCC15.2/15.3/Clangの検査有効/無効で、各約375万件の独立モデルを実行した。最大INT_MAXまでの版・親リンクの確保を確認し、実際のgroupsは1,048,577頂点までgroup数・サイズ・全所属を検証。既存4fixture、ASan/UBSanも確認した（LSan無効）。本体SHAは`0313bdd8c1fbc4bd768e330891145fca69e9205ec7ee07fc9bb6af4673207d04`。

現在のmainも公式14ケース×5設定=70実行で公式期待出力と一致。埋め込みVerifyは検査有効/無効・GCC15.2/15.3/Clang・128bitなし/SIMD無効を含む12構成、使用例2構成に成功した。公式入力と出力は上流commit `73876f1cbcf082d2b22722924b924d3d65a2f00e`のhash.jsonに全件一致。生成器と独立rollback正解の深い再帰だけstackをunlimitedにし、対象ライブラリとVerifyは通常stackで実行した。

旧提出412456のAC・93msは旧ソースの記録として保持する。現行版のオンラインACには読み替えず、ソース更新による再提出待ちとした。

- [提出コード](../Verify/current/submissions/persistent_union_find.cpp)
- [全条件・生データ・不採用案・再現用コード](performance-persistent-union-find-round2-2026-10-30.json)
- [独立参照・公式・ビルド・生成コードの記録](verify-persistent-union-find-round2-2026-10-30.json)
- [既存提出の再確認](online-persistent-union-find-review-2026-10-30.json)
