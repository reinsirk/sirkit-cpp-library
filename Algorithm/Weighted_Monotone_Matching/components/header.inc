// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/weighted_monotone_matching.md
//
// API一覧
// WeightedMatchEdge{left,right,weight}: 両側の位置と選んだときの重み。
// maximum_weight_monotone_matching(edges): 左右とも位置が狭義に増加するように組を選ぶ。
// weightは重みの最大和、indicesは選んだ組の0始まり入力添字（左右の位置順）。
// 空の選択も可。複数解の選択は任意。重みの最大和がint64_tを越えるならoverflow_error。
// 組数n、異なるrightの個数はINT_MAX以下。O((n+1)log(n+2))時間・O(n+1)領域。
