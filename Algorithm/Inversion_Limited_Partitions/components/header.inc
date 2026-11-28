// SPDX-License-Identifier: CC0-1.0
// 参考（転倒数で制限する分割）: https://atcoder.jp/contests/typical90/tasks/typical90_ck
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/inversion_limited_partitions.md
//
// API一覧
// inversion_limited_starts(values,k): 各右端rを含む区間で転倒数<=kを満たす最小の左端を0始まりで返す。
// count_inversion_limited_partitions<Count>(values,k): 各区間の転倒数<=kとなる非空区間への分割数。空列は1。
// 転倒はi<jかつvalues[i]>values[j]の組。等値は数えない。列長はINT_MAX未満。
// Count{}が0、Count{1}が1、+=と-=が使えること。計算途中の累積値もCountで表せること。
// いずれもO(1+n log(n+1))時間・O(n+1)領域。
