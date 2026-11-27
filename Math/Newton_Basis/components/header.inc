// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/newton_basis.md
//
// API一覧
// monomial_to_newton_basis(coefficients, points): 単項式基底からNewton基底へ変換する。
// newton_to_monomial_basis(coefficients, points): Newton基底から単項式基底へ変換する。
// 法998244353。係数数Nは2^20以下、点の数はN-1（N=0では0）。点の重複を許す。
// 両変換ともO(N log^2(N+1))時間・O(N log(N+1)+1)領域。
