// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lexicographic_string_shuffle.md
//
// API一覧
// lexicographic_string_shuffle(words): 各文字列内の順序を保つマージのうち辞書順最小を返す。
// 符号なしバイト順。空文字列も可。n個・総長LでO(n+L log(n+1))時間、O(L+1)領域。
// 総長と空でない文字列数の和はINT_MAX-2以下。超過はチェック有効時にlength_error。
