// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/string_periods.md
//
// API一覧
// StringPeriods(text): 各接頭辞の周期を答えるためにO(n+1)時間・領域で前処理。
// size(): 元の文字列のバイト数。O(1)。
// least_period(length): 先頭lengthバイトの最小周期。末尾は繰り返しの途中で終わってよい。O(1)。
// primitive_root_length(length): 先頭lengthバイトを完全な繰り返しで作る最短のブロック長。O(1)。
// proper_borders(length): 先頭lengthバイトの真の接頭辞かつ接尾辞の長さを降順で返す。長さ0は除く。
// periods(length): 先頭lengthバイトの正の周期を昇順で返す。length自体を含む。
// 列挙はO(k+1)時間・領域で、kは返す要素数。空の接頭辞の最短長は0、列挙結果は空。
// lengthはsize()以下。
