# コンパイラ別配布・速度優先版と追加ライブラリ

12時間作業の成果と確認範囲をまとめます。公開APIとアルゴリズムは各Document、個別の検証・測定はAuditの原記録を参照してください。

## 配置

通常の用途別ソースに加え、`Variants/GCC`・`Variants/Clang`・`Fast/GCC`・`Fast/Clang`を用意しました。通常ソース105本（テンプレートを含む）から計420本を生成します。専用Fast実装は11本で、その他は入力検査の除去を中心にした版です。通常版の安全な所有方式は維持します。

[版の契約と生成方法](../Document/fast_variants.md)を確認し、プログラム全体で一つの版・設定にそろえて使います。FastのNodeだけは生ポインタを公開するため、提出driverにもその変換を用意しています。

## 新規47本

各行の本体は独立したCC0実装です。数学的な構成・機能・問題選定の参考と、コードの出典を区別しています。

| 本体 | 説明 | 使用例 |
| --- | --- | --- |
| [Mo.cpp](../Algorithm/Mo.cpp) | [Document](../Document/mo.md) | [例](../Examples/mo_example.cpp) |
| [Disjoint_Sparse_Table.cpp](../DataStructure/Disjoint_Sparse_Table.cpp) | [Document](../Document/disjoint_sparse_table.md) | [例](../Examples/disjoint_sparse_table_example.cpp) |
| [Dual_Segment_Tree.cpp](../DataStructure/Dual_Segment_Tree.cpp) | [Document](../Document/dual_segment_tree.md) | [例](../Examples/dual_segment_tree_example.cpp) |
| [Dynamic_Bitset.cpp](../DataStructure/Dynamic_Bitset.cpp) | [Document](../Document/dynamic_bitset.md) | [例](../Examples/dynamic_bitset_example.cpp) |
| [Fenwick_Tree.cpp](../DataStructure/Fenwick_Tree.cpp) | [Document](../Document/fenwick_tree.md) | [例](../Examples/fenwick_tree_example.cpp) |
| [Lazy_Segment_Tree.cpp](../DataStructure/Lazy_Segment_Tree.cpp) | [Document](../Document/lazy_segment_tree.md) | [例](../Examples/lazy_segment_tree_example.cpp) |
| [Link_Cut_Tree.cpp](../DataStructure/Link_Cut_Tree.cpp) | [Document](../Document/link_cut_tree.md) | [例](../Examples/link_cut_tree_example.cpp) |
| [Persistent_Segment_Tree.cpp](../DataStructure/Persistent_Segment_Tree.cpp) | [Document](../Document/persistent_segment_tree.md) | [例](../Examples/persistent_segment_tree_example.cpp) |
| [Segment_Tree.cpp](../DataStructure/Segment_Tree.cpp) | [Document](../Document/segment_tree.md) | [例](../Examples/segment_tree_example.cpp) |
| [Segment_Tree_Beats.cpp](../DataStructure/Segment_Tree_Beats.cpp) | [Document](../Document/segment_tree_beats.md) | [例](../Examples/segment_tree_beats_example.cpp) |
| [Sliding_Window_Aggregation.cpp](../DataStructure/Sliding_Window_Aggregation.cpp) | [Document](../Document/sliding_window_aggregation.md) | [例](../Examples/sliding_window_aggregation_example.cpp) |
| [Slope_Trick.cpp](../DataStructure/Slope_Trick.cpp) | [Document](../Document/slope_trick.md) | [例](../Examples/slope_trick_example.cpp) |
| [Sparse_Table.cpp](../DataStructure/Sparse_Table.cpp) | [Document](../Document/sparse_table.md) | [例](../Examples/sparse_table_example.cpp) |
| [Wavelet_Matrix.cpp](../DataStructure/Wavelet_Matrix.cpp) | [Document](../Document/wavelet_matrix.md) | [例](../Examples/wavelet_matrix_example.cpp) |
| [Bipartite_Matching.cpp](../Graph/Bipartite_Matching.cpp) | [Document](../Document/bipartite_matching.md) | [例](../Examples/bipartite_matching_example.cpp) |
| [Cartesian_Tree.cpp](../Graph/Cartesian_Tree.cpp) | [Document](../Document/cartesian_tree.md) | [例](../Examples/cartesian_tree_example.cpp) |
| [Centroid_Decomposition.cpp](../Graph/Centroid_Decomposition.cpp) | [Document](../Document/centroid_decomposition.md) | [例](../Examples/centroid_decomposition_example.cpp) |
| [Eulerian_Trail.cpp](../Graph/Eulerian_Trail.cpp) | [Document](../Document/eulerian_trail.md) | [例](../Examples/eulerian_trail_example.cpp) |
| [Functional_Graph.cpp](../Graph/Functional_Graph.cpp) | [Document](../Document/functional_graph.md) | [例](../Examples/functional_graph_example.cpp) |
| [General_Matching.cpp](../Graph/General_Matching.cpp) | [Document](../Document/general_matching.md) | [例](../Examples/general_matching_example.cpp) |
| [Heavy_Light_Decomposition.cpp](../Graph/Heavy_Light_Decomposition.cpp) | [Document](../Document/heavy_light_decomposition.md) | [例](../Examples/heavy_light_decomposition_example.cpp) |
| [Low_Link.cpp](../Graph/Low_Link.cpp) | [Document](../Document/low_link.md) | [例](../Examples/low_link_example.cpp) |
| [Max_Flow.cpp](../Graph/Max_Flow.cpp) | [Document](../Document/max_flow.md) | [例](../Examples/max_flow_example.cpp) |
| [Min_Cost_Flow.cpp](../Graph/Min_Cost_Flow.cpp) | [Document](../Document/min_cost_flow.md) | [例](../Examples/min_cost_flow_example.cpp) |
| [Offline_Dynamic_Connectivity.cpp](../Graph/Offline_Dynamic_Connectivity.cpp) | [Document](../Document/offline_dynamic_connectivity.md) | [例](../Examples/offline_dynamic_connectivity_example.cpp) |
| [Pruefer_Code.cpp](../Graph/Pruefer_Code.cpp) | [Document](../Document/pruefer_code.md) | [例](../Examples/pruefer_code_example.cpp) |
| [Rerooting_DP.cpp](../Graph/Rerooting_DP.cpp) | [Document](../Document/rerooting_dp.md) | [例](../Examples/rerooting_dp_example.cpp) |
| [SCC.cpp](../Graph/SCC.cpp) | [Document](../Document/scc.md) | [例](../Examples/scc_example.cpp) |
| [Two_SAT.cpp](../Graph/Two_SAT.cpp) | [Document](../Document/two_sat.md) | [例](../Examples/two_sat_example.cpp) |
| [Berlekamp_Massey.cpp](../Math/Berlekamp_Massey.cpp) | [Document](../Document/linear_recurrence.md) | [例](../Examples/berlekamp_massey_example.cpp) |
| [Binary_Matrix.cpp](../Math/Binary_Matrix.cpp) | [Document](../Document/binary_matrix.md) | [例](../Examples/binary_matrix_example.cpp) |
| [Convolution.cpp](../Math/Convolution.cpp) | [Document](../Document/convolution.md) | [例](../Examples/convolution_example.cpp) |
| [Formal_Power_Series.cpp](../Math/Formal_Power_Series.cpp) | [Document](../Document/formal_power_series.md) | [例](../Examples/formal_power_series_example.cpp) |
| [Gaussian_Elimination.cpp](../Math/Gaussian_Elimination.cpp) | [Document](../Document/matrix.md) | [例](../Examples/gaussian_elimination_example.cpp) |
| [Linear_Recurrence.cpp](../Math/Linear_Recurrence.cpp) | [Document](../Document/linear_recurrence.md) | [例](../Examples/linear_recurrence_example.cpp) |
| [Matrix.cpp](../Math/Matrix.cpp) | [Document](../Document/matrix.md) | [例](../Examples/matrix_example.cpp) |
| [Modular_Arithmetic.cpp](../Math/Modular_Arithmetic.cpp) | [Document](../Document/modular_arithmetic.md) | [例](../Examples/modular_arithmetic_example.cpp) |
| [NTT.cpp](../Math/NTT.cpp) | [Document](../Document/ntt.md) | [例](../Examples/ntt_example.cpp) |
| [Pollard_Rho.cpp](../Math/Pollard_Rho.cpp) | [Document](../Document/pollard_rho.md) | [例](../Examples/pollard_rho_example.cpp) |
| [Polynomial.cpp](../Math/Polynomial.cpp) | [Document](../Document/polynomial.md) | [例](../Examples/polynomial_example.cpp) |
| [Xor_Basis.cpp](../Math/Xor_Basis.cpp) | [Document](../Document/xor_basis.md) | [例](../Examples/xor_basis_example.cpp) |
| [Aho_Corasick.cpp](../String/Aho_Corasick.cpp) | [Document](../Document/aho_corasick.md) | [例](../Examples/aho_corasick_example.cpp) |
| [Manacher.cpp](../String/Manacher.cpp) | [Document](../Document/manacher.md) | [例](../Examples/manacher_example.cpp) |
| [Palindromic_Tree.cpp](../String/Palindromic_Tree.cpp) | [Document](../Document/palindromic_tree.md) | [例](../Examples/palindromic_tree_example.cpp) |
| [Suffix_Array.cpp](../String/Suffix_Array.cpp) | [Document](../Document/suffix_array.md) | [例](../Examples/suffix_array_example.cpp) |
| [Suffix_Automaton.cpp](../String/Suffix_Automaton.cpp) | [Document](../Document/suffix_automaton.md) | [例](../Examples/suffix_automaton_example.cpp) |
| [Z_Algorithm.cpp](../String/Z_Algorithm.cpp) | [Document](../Document/z_algorithm.md) | [例](../Examples/z_algorithm_example.cpp) |

## 採用した高速化

- NTTのMontgomery演算・AVX2経路と、それを使う形式的冪級数・畳み込み。
- 永続セグメント木の区間積の再走査削減。
- Wavelet Matrixの構築時の格納・ビット処理。
- 線形漸化式の小さい次数・Clang向け演算経路。
- GCCの32ビット符号なし行列積の分割・再利用。
- 64ビット剰余累乗・素数法平方根の演算器共有と直接式。
- Fastの生ポインタ・プール・格納方式・破棄省略のうち、測定で選んだ実装。

P-CoreのCPU12でGCC15.2／Clang21.1.8、C++23・O2、既定／nativeをそろえて比較しました。main・入出力を変更した効果は本体の成果に含めません。小入力で数nsの後退が残る条件を個別Auditへ記録しています。すべての条件で速いという主張や、オンラインFastestの達成を意味しません。

[採用ソースと測定証拠の対応表](final-performance-evidence-review-2026-10-31.md)に、通常7ソース・Fast11本の対象SHA、注記だけの差分、主要な退行、保存した再現用資料をまとめました。
[代表条件の速度表](accepted-performance-table-2026-10-31.md)では、比較元・入力長・GCC/Clangの4設定・残る退行を並べて確認できます。

Link Cut Treeの小型ノード・破棄省略は、照会の利益が測定の変動範囲で、一部操作が退行したため不採用です。コピー・構築を繰り返す場合、解放省略が新しいページへの書込みを増やす点も比較しました。

[LCPの補助配列の初期化省略](lcp-rank-overwrite-rejected-2026-10-31.json)も、1440測定では大規模・実mainの利益が一貫せず、空入力の確保が増えたため不採用です。現行のC++本体は変更していません。

## 検証

| 範囲 | 結果と資料 |
| --- | --- |
| 通常版227コード | [1,816構成・3,184入力例](normal-verify-final-release-sqrt-2026-10-31.json)。GCC／Clang・検査あり／なし・既定／標準代替 |
| 各配布版の提出コード・使用例 | [2,840構成](variant-programs-local-validation-2026-10-31.json)。各凍結コードとcheckerのSHAを保持 |
| コンパイラ分岐・標準代替経路 | [1,470構成](verify-compiler-variants-current-2026-10-31.json) |
| 宣言の同時利用 | [72構成](verify-library-combinations-current-2026-10-31.json)。逆順・二重貼付・2翻訳単位を含む |
| 16ライブラリの実行併用 | [330実行・304種類の構成](runtime-coexistence-2026-10-31.json)。対象ソース・版のSHAを限定した検証 |
| SIMD選択 | [5本・20条件](runtime-simd-dispatch-2026-10-31.json)。検査用の機能判定・カウンタを使用し、未対応CPUの実機試験とは区別 |
| 最大長CRT畳み込み | [50,331,648係数を照合](convolution-maximum-capacity-2026-10-31.json)。長さ2^23・最大32ビット法・巨大な符号付き値の相殺 |

本体を凍結した5種類のZIPは、CRC・版のSHA・貼付コード・資料リンクの独立検査を通過しました。[使用例618構成の結果](source-frozen-distribution-2026-10-31.json)は、アルゴリズムの独立参照検査と区別します。配布フォルダの`manifest.json`には、最終ZIPそのもののSHAと最終検査の結果を置きます。ZIP自身のSHAを内部へ書き戻す循環は作りません。

[空フォルダでの再現検査](cold-distribution-gate-2026-10-31.json)では、同梱ツールによる126テスト・14同期チェックが通り、5種類のZIPをバイト単位で再現できました。Python・zlibの版を固定した候補版の記録であり、その後の資料追加を含むZIPのSHAと区別します。

[正規入力からの再生成](canonical-regeneration-gate-2026-10-31.json)も、生成先1,488ファイルを退避して実行し、候補ZIPの全3,586ファイルと一致しました。FastのNodeと内部3部品は手書きの正規入力として保持し、提出URLの記録とC++本文を別々に確認しています。

[読みやすい連結ツール](../Document/bundling.md)も追加しました。同一の共有部品だけを省き、NTT・冪級数・多項式・任意法畳み込みの通常版4本は190,740から85,732バイトになります。[72構成の前処理比較と36構成の実行確認](text-bundler-2026-10-31.json)、[別担当の10組の実行比較と18項目](text-bundler-independent-review-2026-10-31.json)を別々に記録しています。

[実ZIPからの利用手順](zip-bundler-usage-2026-10-31.json)でも、`--root`指定なしで5版を生成し、生成テキストを使う数学APIの20条件を確認しました。[保存物参照の検査](final-artifact-reference-check-2026-10-31.md)は認識した248参照・180ファイルの存在・記載サイズ・SHAに限定し、数式検証と区別しています。

現在の提出コードは通常版最大392,209バイト、派生版最大384,881バイトで、1,029本とも512 KiB以内です。ライブラリを全て同時に貼り付ければ上限を超えるため、必要な機能を選びます。連結ツールも既定で512 KiBを超える出力を拒否します。

## 出典と提出

[47本とFast上書きの確認](final-policy-provenance-47-libraries-2026-10-31.json)は対象SHAを固定した記録です。その後のModularArithmeticは、[平方根の採用記録](modular-sqrt-adoption-receipt-2026-10-31.json)と[最終本体への注記だけの差分](modular-sqrt-source-comment-bridge-2026-10-31.json)で補足しています。既存GenericHashのSplitMix64 finalizerは原典のPD提供声明・無条件許諾を区別し、[原文の通知](../LICENSES/SplitMix64-Public-Domain.txt)も同梱します。

通常版の今回追加79本と、本体を変えた既存HashMapを[提出一覧](submission-checklist-12hours-2026-10-31.md)にまとめました。以前からの未提出・再提出記録を今回の変更と混ぜません。ローカル検証はオンラインACの代わりではなく、各配布版へ通常版のACは転記していません。

[新規79本のdriver読解レビュー](verify-new-driver-domains-2026-10-31.md)も行いました。APIと問題の変換に確認できた実不具合はありませんでしたが、公式本文2件・一部上限の取得未完と、別解を許す問題の固定出力回帰検査は範囲を明記しています。
