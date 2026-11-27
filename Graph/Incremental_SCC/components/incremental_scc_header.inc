// SPDX-License-Identifier: CC0-1.0
// 参考（オフライン版の時刻分割・縮約）: https://doi.org/10.4230/LIPIcs.SWAT.2026.17
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incremental_scc.md
//
// API一覧
// incremental_scc_merges(n,edges): 空の有向グラフに全辺を入力順で追加したときのSCC併合を時刻順に返す。
// IncrementalSccMerge{time,leader,absorbed}: time本目の追加後に、absorbedを代表とする成分をleaderに併合する。
// 代表の初期値は自分自身。同じ時刻のイベントは結果の順に適用する。頂点番号は[0,n)、時刻0では併合なし。
// 自己辺・重複辺可。時刻分割と縮約でO(n+m log(m+1))時間、O(n+m)領域。
// 1時点の成分分解にはscc_graph、全追加が既知の場合の併合列にはincremental_scc_mergesを使う。
