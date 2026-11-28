// SPDX-License-Identifier: CC0-1.0
// 参考（左端の一致を繰り返し削除する問題）: https://atcoder.jp/contests/abc328/tasks/abc328_d
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pattern_erasure.md
//
// API一覧
// erase_pattern(text,pattern): 左端の連続一致を繰り返し消した文字列を返す。
// erase_pattern_inplace(text,pattern): textを書き換え、削除された出現回数を返す。patternがtext内を参照してもよい。
// バイト単位で比較し、空patternはinvalid_argument。どちらもO(n+m+1)時間・O(n+m+1)領域。
