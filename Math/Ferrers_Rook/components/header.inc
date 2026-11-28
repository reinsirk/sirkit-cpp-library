// SPDX-License-Identifier: CC0-1.0
// 参考（Goldman–Joichi–Whiteの積公式と包除原理）: https://www.combinatorics.org/ojs/index.php/eljc/article/download/v15i1r85/pdf/
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/ferrers_rook.md
//
// API一覧
// ferrers_rook_counts(heights): 同じ行・列を使わずにk個置く方法の数を、k=0..nの順で返す。
// 各列の許可マスは先頭からheights[i]個。長方形の盤も扱い、列の並び順は任意。
// ferrers_hit_counts(heights): n×n盤で順列の選んだnマスのうち、許可マスがk個ある順列数を返す。
// heights[i]>nならinvalid_argument。nは列数で2^20未満。空列に対する結果は両方{1}。
// 法998244353。O(1+n log^2(n+1))時間、O(1+n log(n+1))領域。
