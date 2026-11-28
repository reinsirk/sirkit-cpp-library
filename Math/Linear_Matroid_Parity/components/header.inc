// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/linear_matroid_parity.md
// 参考（アルゴリズム）: https://atcoder.jp/contests/abc482/editorial/27996
//
// API一覧
// LinearMatroidParity<Field>(dimension): D次元の有限体ベクトルを受け付ける。O(D^2)時間・領域。
// add_pair(left,right,coefficient): 疎なベクトルの対を追加。要素は{index,value}、同じ座標は加算。
// 列挙要素数がa,bならO(a+b+ab)時間。行列の添字は0始まり。範囲外はout_of_range。
// sample_size(): 合わせて線形独立になる対の最大数の下界を返す。O(D^3)時間・O(D^2)追加領域。
// 各対の係数を位数qの体から独立一様に選ぶ。真の最大数がrのとき、過小評価の確率はr/q以下。
