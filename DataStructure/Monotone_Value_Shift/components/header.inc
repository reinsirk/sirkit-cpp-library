// SPDX-License-Identifier: CC0-1.0
// 参考（値の範囲で区間を選ぶ用途）: https://atcoder.jp/contests/abc389/tasks/abc389_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/monotone_value_shift.md
//
// API一覧
// MonotoneValueShift(values): 昇順のint64_t列を保持。O(n+1)時間・領域。n<=INT_MAX、空構築可。
// size()/empty(): 長さ/空か。O(1)。get(i): 0始まりの値。O(log(n+1)+1)。
// lower_bound(value)/upper_bound(value): value以上/より大きい最初の位置。なければn。
// count_between(low,high): 閉区間[low,high]内の値の個数。low<=high。
// shift_between(low,high,delta): 変更前の値が[low,high]に入る全要素をdeltaずらす。昇順とint64_t範囲を保つこと。
// 境界探索・個数・更新はO(log(n+1)+1)。values(): 現在値を全て返す。O(n+1)時間・領域。
// 検査有効時は不正区間・昇順を崩す更新・数値範囲超過で例外。更新失敗時は元の値を保つ。
