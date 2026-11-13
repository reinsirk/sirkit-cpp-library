# 版を選ぶための代表速度表

比率は **after / before**（小さいほど速い）。各セルの順は **既定 / native**。Intel Core i9-14900KのCPU12（P-Core）、GCC15.2／Clang21.1.8、C++23・O2・SKIP_RUNTIME_CHECKSでの既存記録。nativeだけ`-march=native`を追加した。通常版への今回の高速化はFastにも継承されている。

新規ライブラリのbeforeは、今回の作成途中の基準実装です。既存公開版の提出時間と区別し、各比較元はJSONで特定します。

| 対象 | 代表条件・測定範囲 | GCC 既定 / native | Clang 既定 / native | 主な後退・注意 |
| --- | --- | --- | --- | --- |
| NTT / FPS | 法998244353、cache準備済み524288×524288のnormalized_move畳み込み。body threadCPU、1畳み込みのμs | 0.401 / 0.381 | 0.376 / 0.389 | 長さ16逆変換でClang+17.4ns。FPS単体の数値は未測定。 |
| PST（通常 / Fast） | n65536、prod120000回、operation1（更新なし）。prod phase threadCPU比 | 通常0.499/0.478、Fast0.474/0.488 | 通常0.523/0.531、Fast0.544/0.528 | 構築・破棄の小区間は後退もある。Fastが常に通常版より速いわけではない。 |
| Wavelet Matrix | 既定:31bit n1000000構築。native:B64 n100000/500000、3照会API各200000回。既定は子CPU全体、nativeは照会body | 構築0.182 / 照会0.999–1.014 | 構築0.266 / 照会0.974–1.010 | 既定/nativeで代表指標が違う。短いB64では一時+21%とcontrol変動があり長い測定へ。 |
| LinearRecurrenceWeights | uint32係数mod998244353、次数256、k=UINT64_MAX。body threadCPU ns / 照会 | 1.001 / 1.000（専用対象外） | 0.757 / 0.761 | 空入力に微小な負荷増。BM/Term/Prefix全体を高速化した数値ではない。 |
| Matrix<uint32_t> | 16×1024 と1024×1024 の積、unsigned mod2^32。body threadCPU μs / 積（同じ積5回の平均） | 0.197 / 0.190 | 該当外（同一code） | 旧full-stride候補の4096幅退行は不採用。小差・子wallの妨害をCPU値と区別。 |
| ModPow / ModSqrt | Pow:最大64bit素数・乱数指数16384件。Sqrt:Goldilocks法の平方剰余8192件。body threadCPU μs / 記載件数 | Pow0.608/0.618、sqrt0.736/0.731 | Pow0.609/0.636、sqrt0.742/0.724 | Pow指数1で+.94ns、Clang p17 sqrtで+.5–1.3ns。先行Powの全条件を最終Sqrt後に再測定していない。 |
| Fast Node | 1000000ノード構築+chain_size2回+破棄、mode0。phase合計threadCPU ns（OS終了回収を除く） | 0.124 / 0.126 | 0.123 / 0.122 | 生ポインタAPI・共有所有なし。旧new-per-nodeのpush/pop退行をpoolで回避。 |
| Fast BinaryTrie（既定） | uint32/MAX_LOG29、131072件の混合操作、n条件200000。phase合計threadCPU ns（OS終了回収を除く） | 0.912 / 0.899 | 1.029 / 0.907 | Clang既定はこの代表条件+2.9%。初回ID操作単体の退行も残る。inline-IDは別オプション。 |
| Fast HashMap / HashSet | 10個のlifecycle条件、n32768/262144、reserve/成長→追加→照会→破棄。子CPU全体（user+system）比の10条件中央値 | 0.804 / 0.801 | 0.802 / 0.798 | 列挙/空予約だけ等の後退はある。空予約だけの25–35倍を全体の効果としない。 |
| 他Fast6種（F/S/L/W/T/A） | Fenwick/Segment/Lazy/Wavelet/Treap:n100000,Q200000。Aho:text500000,patterns50000。子CPU全体ns、各自の通常版をbeforeとした6種の範囲 | 0.965–1.019 / 0.961–1.010 | 0.831–1.029 / 0.846–1.021 | 大半は中立。Treapの追加利益は約1%、AhoはGCCだけUnreleased採用しClangの+17–20%切替は不採用。 |

Fastのphase合計は構築・操作・破棄のCPU時間の和で、OS終了時の漏洩領域回収は含まない。子CPU全体は起動から終了までのuser+system。異なる測定範囲を横に比較しない。Fastは破棄・解放・例外保証を省く契約があり、通常版をそのまま置換できないAPIもある。

PSTの通常/Fastはともに同じ旧通常版に対する比。ModPowのbeforeはC版、SqrtのbeforeはMontgomery導入後の版で別基準。Waveletの既定は構築、nativeは長い照会群で、未測定のnative構築値を補間していない。Fast6種の最後の行は全6種の範囲であり、各種の具体値はJSONにある。

元Auditのキー・入力長・before/after・各4設定の値は[条件と数値](accepted-performance-table-2026-10-31.json)。この表は再測定・オンライン判定・すべての入力での優位を意味しない。
