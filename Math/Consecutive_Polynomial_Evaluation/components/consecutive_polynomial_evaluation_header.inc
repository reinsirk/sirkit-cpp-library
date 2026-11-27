// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/consecutive_polynomial_evaluation.md
//
// API一覧
// evaluate_consecutive_polynomial(samples,first,count): f(0)..f(n-1)からf(first)..f(first+count-1)を返す。
// 次数n未満・法998244353。n=0は零多項式、firstはint64_tで負も可。n,count≤2^20。
// nはsamples.size()、m=count。O(1+(n+m)log(n+m+1))時間・O(1+n+m)領域。
