// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/l1_origin_line_fit.md
//
// API一覧
// l1_origin_line_fit(points): 原点を通る直線へのマンハッタン距離の総和を最小化する。
// pointsは{int32_t x,int32_t y}の列。戻り値L1OriginLineFitは{cost,slope,swapped}。
// swapped=falseはy=slope*x、trueはx=slope*y。|slope|<=1。costとslopeは既約Fraction。
// O(n log(n+1))時間、O(n+1)領域。空列の答えは費用0・傾き0・swapped=false。
// nはINT_MAX以下。最小費用を既約にした分子がint64_tに収まらなければoverflow_error。
