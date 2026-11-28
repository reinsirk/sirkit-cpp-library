// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_shift_minimum.md
// 参考（円環上の各区間を数える帰着）: https://atcoder.jp/contests/arc182/editorial/10477
//
// API一覧
// sum_minimum_modular_shifts(count,modulus,step,offsets,start=0):
// k=0..count-1に対し、(start+step*k+offset) mod modulusのoffsetに関する最小値を取り、全kで総和。
// 結果はuint64_tで厳密。countとmodulusはuint32_t。step/start/offsetは負のint64_tも可。
// modulus=0または空のoffsetsはinvalid_argument。有効な入力でcount=0なら0。
// offsetsがn個、剰余が異なる値がu個ならO(n log(n+1)+u log(modulus+1))時間・O(n)領域。
// count=1またはstepがmodulusの倍数ならO(n)時間。
