// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_fenwick_tree.md
//
// API一覧
// RangeFenwickTree<T=int64_t>(n=0)/RangeFenwickTree(values): n個の0または既存の列で構築。O(n+1)時間・領域。
// add(l,r,delta): 半開区間[l,r)にdeltaを加える。sum(l,r): 同じ区間の和を取得する。
// get(i)/set(i,value): 一点の取得・代入。size()/empty(): 長さ・空判定、O(1)。
// 更新・照会はO(log(n+1))。添字は0始まり。Tはboolを除く64ビット以下の整数型。
// 整数演算はTのビット幅で剰余を取る。答えがTの範囲に収まる場合は通常の加算・区間和と一致する。
