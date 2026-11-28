// SPDX-License-Identifier: CC0-1.0
// 参考（等間隔のシフトの重み付き和を求めるVerify）: https://yukicoder.me/problems/no/3793
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/weighted_taylor_shifts.md
//
// API一覧
// weighted_taylor_shifts(f, shifts, weights): Σ weights[i]*f(x+shifts[i])をfと同じ係数数で返す。
// 法998244353。シフトと重みは整数列で、負数や同じシフトも可。個数が異なればinvalid_argument。
// シフトが空なら全係数0、fが空なら空。fの係数数dは2^22以下。
// シフトm個に対してO((m+d+1)log²(m+d+2))時間、O(1+m+d)領域。
