// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/maximum_weight_window.md
//
// API一覧
// WeightedWindowPoint{x,y,weight}: int32_tの点の座標とint64_tの重み。座標の重複・負の重みも可。
// maximum_weight_window(points,width,height): 幅・高さが正の半開長方形で重みの和を最大化。
// {weight,left,bottom}を返す。最適な空の長方形も可。左端・下端は整数。
// n点でO(n log(n+1))時間、O(n+1)領域。重みの絶対値の和はINT64_MAX以下、幅と高さはuint32_t。
// 端点数はintに収まること。範囲外の幅・重み・端点数は例外。
