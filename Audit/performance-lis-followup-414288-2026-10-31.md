# LIS提出414288の確認と終端比較の追加改善（2026-10-31）

[414288](https://judge.yosupo.jp/submission/414288)は全19ケースAC・最大27ms。前回414139の最大24msから増えたが、max_randomの5件は全て20msへ短縮した。一方、random01/03は22/21→27/27ms、hand_minは7→16ms、example00も1→9ms。公開APIの本文はCRLF/LF・空行・行端空白・Submission注記だけの正規化で、旧2e97本体・main・共通I/Oと全文一致した。

オンラインで増加した4ケースと比較用のmax_random04を、同じmain/I/O・GCC15.2・P-Coreで旧687eと2e97に対して21反復した。random01/03は通常/nativeとも約20%速く、hand_minも約1.3%速い。example00の増加も再現しない。**オンラインの増加原因は特定できない。** 実装の後退を示すローカル再現が得られないため、2e97の高速化は維持する。judgeと同じCPU・実行条件だとは説明しない。

## 採用

AVX2で最後の候補が8〜15件となった際、先頭8件と末尾8件を重ねて独立に比較する。先頭maskにfalseがあればその先頭位置、全trueなら末尾側の位置を使う。候補全体を被覆するため、残りの二分探索へ戻る必要がない。4〜7件は従来の先頭4件比較と残り二分探索を使う。値の減算はなくsigned64の比較を保つ。全loadはlo..lo+count-1内で、size外を読まない。

GCC対象の段階探索helperへalign-loops=32属性を加え、非選択経路でのループ配置差も比較した。64byte指定は交互列の後退があり採用しない。selector、所有権、公開API、検査、前駆復元、計算量/追加領域は変更しない。mainと共通I/Oも完全に固定した。新たな外部実装を参照せず、既存CC0を維持する。

最終SHAは`0331d3b8305d028d69e78f7c4f510d72953358daa198f60d676e135a2005a32f`。対象は従来どおりGCC15.2/x86-64・狭義int64の専用経路。SIRKIT_DISABLE_SIMD/Clang/GCC15.3では旧2e97と前処理一致し、CPU未対応時も標準経路へ戻る。

## 測定

i9-14900KのP-Core（論理CPU12）、GCC15.2/C++23/O2/検査省略。通常とnativeを別計測し、同一旧binaryのcontrolを加えseed固定shuffle。同じpath/inodeへcopyして実行する。copy/入力作成/出力照合は計時外。APIはprocess CPU時間、mainは子user+system CPU時間とwallを別記録。

固定mainで公式19ケースを9反復し、caseごとのCPU時間中央値を合計した結果：

| 設定 | 414288の本体 | 今回の本体 | 短縮率 |
|---|---:|---:|---:|
| gcc | 123.555 ms | 111.422 ms | 9.82% |
| native | 123.137 ms | 117.245 ms | 4.78% |

50万要素randomの復元単体は通常約19.7%、native約9.3%短縮した。これは固定mainの改善率やオンライン最大時間とは別指標。

公式19件＋補足35件、2設定・3variant・9反復の2916実行は全出力一致。各初回を公式checker、または独立bisect長と添字/値順序で照合。int64全域・2〜1024種類・周期/交互・長い延長・段階/cache境界を含む。

懸念ケースをさらに21反復した。50万4値は通常+0.04%、native-0.72%、周期列は通常約9.2%、native約10.0%短縮。max_random04も通常約12.5%、native約5.5%短縮した。通常8値で約1.05%増（同じ旧binary controlも約1.48%増）、nativeのlowprefix_rampで約0.62%増（control約0.67%増）など微差は残る。**全入力・別programでの不退行やオンライン最大時間の短縮は保証しない。** rawとcontrolを含めてJSONへ保存する。

## 検証と配布

最終本体を4本の独立参照fixtureで64構成確認した。GCC15.2/Clang通常/native・検査有無、GCC15.2 SIMD無効通常/native、GCC15.3、ASan/UBSanを含む。ASanには_GLIBCXX_SANITIZE_VECTOR=1を加えてlogical size外も検出し、LSanは除外、UBSanはエラー時停止。compile/run stderrは空。CPU判定だけfalseにした別sourceの8構成も成功し、標準経路の分岐を模擬した。

候補3案の終端kernelには独立lower_bound照合142,956件、Fenwick参照59ケース・両Index経路の照合、ASan/UBSanを実施。overlap_allは非選択4値列で後退、terminal2は改善が小さいため不採用にした。

長さ・接頭辞・末尾長・広義・int32の10組のbinaryは旧版と5section一致。生成提出・使用例・相対includeもGCC/Clang・検査有無の20構成で通り、オンラインdriverは公式19ケースを再確認。生成最終版と測定版も実行code/定数/data/例外領域が一致した。

本体・Document・Verify・ZIPを同期する。414288のAC27msは旧2e97で、今回0331は未提出。再提出対象は`Verify/current/submissions/lis.cpp`の1本。
