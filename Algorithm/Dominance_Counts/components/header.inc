// SPDX-License-Identifier: CC0-1.0
// 参考（組の条件の用途）: https://atcoder.jp/contests/abc231/tasks/abc231_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dominance_counts.md
//
// API一覧
// dominance_counts(points,strict_x=false,strict_y=false,x_less={},y_less={}): 各点に対し両座標でそれ以前の点の個数。
// points[i].first/secondを比較し、既定は昇順。strict_x/yが真なら対応軸で同値を数えない。
// 入力順にintの列を返す。重複点は別々に数え、両方非狭義なら自身も含む。点数はINT_MAX以下。
// 比較がO(1)のときO(1+n*log(n+1))時間・O(n+1)領域。比較は狭義弱順序を成すこと。
