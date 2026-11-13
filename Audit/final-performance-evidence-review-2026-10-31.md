# 採用高速化と最終ソースの対応

通常7ソースとFast11上書きを読み取りで照合した。新しい性能測定、C++本体の変更、広い再検証は行っていない。最終summaryは「全条件で高速」「オンラインFastest達成」を明示的に否定しており、過大主張として取り消すべき記述は見つからなかった。

| 対象 | 最終版との対応 | 条件・残る制限 |
| --- | --- | --- |
| NTT / FPS | NTT採用SHAと現行が一致。測定候補との差は注記のみ。FPSにも同じNTT本文を正確に同梱し、FPS本体の部品は従来と同一 | 9,280行、CPU12・GCC/Clang・O2・既定/native。Clangの長さ16逆変換は+17.4ns。FPS各操作を独立に数値測定したという意味にはしない |
| PersistentSegmentTree | 通常/Fastとも採用SHAと現行が一致。候補との差はAPI/所有権説明だけ | 792行。選択したprodは改善するが、構築・破棄の個別小区間には後退も記録 |
| WaveletMatrix | 現行SHA一致。最終volatile判定の既定ビルドを別途測定、nativeは従来候補との機械語同一性を記録 | 短いB64の+21%を黙認せず長い5反復で再確認。nativeの小差はcontrol変動と区別。実際の未対応CPUでの実行試験ではない |
| LinearRecurrence | 採用SHA一致、候補との差は先頭注記のみ | 1,568行。Weightsの初期範囲shortcutとClangのd>=16専用経路が対象。BM/Term/Prefix全部の高速化ではない。微小な空入力の負荷増を記録 |
| Matrix | 測定したtile32/row4候補と現行が完全一致 | 4,590行。GCCのuint32のみ。4096幅で退行した旧候補は不採用。全子プロセスwallは一部の妨害/起動時間で悪化し、body/thread CPUと別保存 |
| ModPow / ModSqrt | Sqrt選択版→現行はAPI注記だけ。先行Powから公開Pow本体は不変で、最終Sqrt測定にもPow control4条件を含む | Powの短い指数1で+.94ns、SqrtのClang p17非平方剰余で+.5–1.3nsを記録。先行Powの全条件を現行で再測定したという意味にはしない |
| Fast Fenwick / Segment / Lazy / Wavelet | 各現行ソースは記録したFastBuffer候補と一致 | FastStorage3,024行およびUnreleased比較1,440行。通常操作は概ね中立で一部+1–3%。破棄省略が全照会を速めるとは主張しない |
| Fast Node | 現行との差はpush_front説明の`new`→`make_node`だけで本文同一 | 採用poolを含む7,056行。1ノードずつnewする旧案のpush/pop退行と、DCEされる初期fixtureは採用の根拠から除外 |
| Fast BinaryTrie | 現行採用SHA一致。既定/inline-IDの各経路がそれぞれ測定候補とプリプロセス後同一 | 基本3,696行・inline比較3,024行。既定の初回IDだけの操作は大きな倍率の退行があり、全体費用やオプションのメモリ増と区別 |
| Fast HashMap / HashSet | 各現行との差はSplitMix出典の先頭2行だけで本文同一 | 14,400行。大規模lifecycleの子CPU中央値は約0.80倍。一方、列挙や空予約の一部は後退。空予約だけの大きな改善を追加・照会込みの改善と混同しない |
| Fast Treap / Aho | Treapは最終SHA一致。Ahoの未使用helperガード追加は実際に測定したbinaryとのruntime-section bridgeで対応 | Treapの追加利益は約1%程度。AhoのUnreleasedはGCCだけ採用。Clangで+17–20%になる切替は不採用。小さいRSS値はPython親のメモリfloorと区別 |

元Audit、全SHA、差分scope、raw件数、退行・未確認は[照合結果](final-performance-evidence-review-2026-10-31.json)に個別記録した。既存の測定scopeや過去ACを現在版へ書き換えていない。

重要なsession内証拠は[補足ZIP](data/accepted-performance-sources-2026-10-31.zip)へ保存した。575ファイル、展開57.3MB、ZIP14.46MB。NTT/FastStorageの候補・基準・正確なfixture・flags・raw/summary、他の採用候補のsource/入力等を含む。コンパイル済みbinaryは含めない。全内容の元path/SHA/版scopeはZIP内manifestに記録した。

補足ZIPをfreshな場所へ展開し、同梱のreplayからNTT採用候補GCC/default1本とFast Fenwick GCC/default1本を別の出力先へ[ビルドできた](data/accepted-performance-replay-2026-10-31.json)。これは再配置のビルド確認だけで、新しい性能値や全575ファイルの実行確認ではない。
