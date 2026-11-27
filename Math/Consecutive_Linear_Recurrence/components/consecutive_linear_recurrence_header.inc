// SPDX-License-Identifier: CC0-1.0
// 数学的参考: A. Bostan, R. Mori, A Simple and Fast Algorithm for Computing the N-th Term of a Linearly Recurrent Sequence, §3
// (https://specfun.inria.fr/bostan/BoMo21.pdf)。係数区間の復元は独立実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/consecutive_linear_recurrence.md
//
// API一覧
// consecutive_linear_recurrence(initial, coefficients, first, count): 法998244353で第first項から連続count項を返す。
// 両入力の長さdは等しく、a_i=Σ_j c_j*a_(i−j−1) (0≤j<d、i≥d)。各係数は剰余に直す。
// count=0なら空、d=0なら0列。最後の添字がuint64_tを超える場合はoverflow_error。
// NTT用の上限はd+count≤2^22（count>0のとき）。超過はlength_error、通常版で長さ不一致はinvalid_argument。
// O(d log(d+1) log(first+2)+(d+count) log(d+count+1))時間、O(d log(first+2)+count)領域。
