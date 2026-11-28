// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/interval_cover_subsets.md
//
// API一覧
// IntervalCoverChoice<T>{left,right,omit=1,take=1}: 半開区間[left,right)と、未選択・選択時の係数。
// count_interval_cover_subsets(left,right,choices): [left,right)全体を覆う選び方の重み総和。
//   重みは各区間の係数の積。同じ区間の重複も別個に選ぶ。係数を省くと個数。
//   座標は int64_t。T{} と T{1} は加法・乗法の単位元。加算・乗算は可換で分配法則を満たすこと。
//   空の対象区間はすべての部分集合を数える。逆向き区間は例外。O(1+m log(m+1)) 時間・O(m+1) 領域。
//   現在の連続被覆の右端を状態にする。左端順に走査し、同じ右端に到達する選択を集計する。
