// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bounded_nondecreasing_sequences.md
//
// API一覧
// count_bounded_nondecreasing_sequences(lower, upper): lower[i]<=x[i]<upper[i]を満たす広義単調増加列の個数 mod 998244353。
// Nは両列の長さ（同じ）、Mは条件を満たし得る値の幅。N+M<=2^21が必要。
// 空の列は1通り。条件が矛盾すると0。O((N+M)log²(N+M+1))時間、O(N+M)領域。
