// SPDX-License-Identifier: CC0-1.0
// 参考（接尾辞配列による回転対の数え上げ）: https://atcoder.jp/contests/abc272/editorial/4988
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cyclic_rotation_order.md
//
// API一覧
// cyclic_rotation_ranks(text): 各巡回シフトの辞書順順位。0始まりで同じ回転は同順位、欠番なし。
// cyclic_rotation_ranks(texts): 同じ長さnのm個の文字列を一緒に順位付け。結果[i*n+j]は第i文字列の開始位置j。
// cyclic_rotation_order(text): 開始位置を回転の辞書順に並べる。同順位では開始位置の小さい順。
// count_cyclic_rotation_pairs(first,second): 同じ長さの2列についてfirstの回転<=secondの回転となる開始位置対の個数。
// 符号なしバイト順。空文字には開始位置がなく、結果は空または0。2*m*n<=INT_MAX-2。
// 複数列の順位はO(m*n+m+1)時間・O(m*n+1)領域。他はO(n+1)時間・領域。
