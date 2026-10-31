# LIS提出414289の確認と追加案の比較（2026-10-31）

[414289](https://judge.yosupo.jp/submission/414289)は全19ケースAC・最大20ms。前回414288の最大27msから短縮し、大規模random5件も20→18/19/19/18/19msになった。random01/03は27→17/16ms、hand_minは16→8ms、example00は9→0ms。公開API本文はCRLF/LF・空行・行端空白・Submission注記だけを正規化すると、現行0331版・main・共通I/Oと全文一致した。

## 結論

本体は0331版を維持し、追加の再提出は不要とする。今回の案は正しさの検証が通るものの、通常設定の改善が小さいか、大規模な別条件で後退が残る。既存の高速化と汎用API・所有権を保つことを優先し、実装へ取り込まない。mainと共通I/Oは完全に固定した。高速化が不可能という結論ではなく、今回比較した範囲で採用できる案がなかったという意味。

## 比較条件

i9-14900KのP-Core（論理CPU12）、GCC15.2/C++23/O2/検査省略。通常とnativeを別々に測定。同じ旧binaryのcontrolを含め、seed固定shuffle順で同じ実行path/inodeへcopyする。copy/入力準備/正誤照合は計時外。APIはprocess CPU時間、mainは子user+system CPU時間とwallを別記録。提出先のnative指定は仮定しない。

公式19件と補足35件を各variant/設定で5反復し、caseごとの中央値の合計を比較した。各初回は公式checkerまたは独立bisect長と添字/値順序を確認し、全出力が一致した。

## 不採用案

- maskを元の添字へ重ねてORし、countr_oneを1回にする案：通常mainは約0.8%増、nativeは約6.4%短縮。重複を連結し補正する案も通常約0.7%増、native約6.0%短縮。通常設定での改善がない。
- AVX2とBMI2の両対応CPUで別workerを選ぶ案：旧AVX2 workerを保ち、CPU検査で新workerを選ぶ。通常mainはほぼ同等、nativeは約6.9%短縮だが、周期/交互列の後退と約6.8KiBのsource増加がある。
- BMI1も確認し、Alder Lake向けの命令配置を指定する案：通常mainは約1.0%短縮したが、同じ旧binary controlにも約0.4%短縮が見える。nativeは約6.1%短縮する一方、周期列が約4.2%増などの後退が残る。
- 増加prefixを認識して前駆/末尾配列を遅れて作る案：完全増加のhand_maxは約15〜16%短縮し、近い整列入力でも改善がある。ただし周期列で通常約3.4%、native約6.2%増が残る。
- 全増加ならiotaを返し、失敗時は旧Phasedへ戻すwrapper：旧本文をbyte同一で保てるが、長いprefixの再読が追加される。24補足ケース・7反復（1344実行）で確認すると、末尾だけ逆転した50万要素列でnative約9.7%増、末尾0/中央値等でも約5%増がある。
- BMI2とwrapperの組合せ：通常main約2.3%、native約7.2%短縮するが、周期/交互列等で約3〜6%の後退が残る。

中央値合計とオンライン最大時間は別の指標。controlや呼出し元・code配置の影響も含め、rawをJSONへ保持する。

## 独立検証

mask2案は独立lower_bound kernelとFenwick参照、checked/skip、ASan/UBSan（vector容量poison有効）の12構成すべて成功。mask代数100通りも全列挙した。BMI2候補も新workerを直接呼ぶ6構成が成功し、旧AVX2 workerのbyte同一を確認した。

prefix案とwrapper案はそれぞれ独立318ケース・約1611万照合、確保失敗・入力不変・生存確保差0、既存Fenwick/cache、ASan/UBSanを含む8構成が成功した。各案の対象外6前処理条件も0331と一致した。tuned/合成版は性能比較中の出力照合までで、専用のASan全構成を済ませたものとは扱わない。

試作コードを合成する際に内部helperの取込み範囲を誤った2件のcompile失敗、およびprefix fixtureの生存vectorを残した検査不備は、成功結果から分離して記録した。本体の不具合ではなく、修正後・凍結後の試験だけを有効とする。

本体はsource SHA `0331d3b8305d028d69e78f7c4f510d72953358daa198f60d676e135a2005a32f` のまま。以前の64構成＋CPU分岐8構成は同一sourceの検証として保持し、今回同じmatrixを重複実行・加算しない。新しいonline AC20ms、最新URL、比較記録を保存し、本体・使用例・Verify・ZIPの同期を確認する。
