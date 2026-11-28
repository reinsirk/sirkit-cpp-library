// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/substring_index.md
//
// API一覧
// [SubstringIndex] 固定したバイト列の部分文字列を索引化する。重なる出現も開始位置で区別する。
// SubstringIndex(text): 文字列を所有して構築。長さはINT_MAX-1未満。O(n+1)時間・領域。コピーは索引を共有。
// distinct_substring_count(): 空を除く相異なる部分文字列数。O(1)。
// kth_distinct_substring(rank): 辞書順で1始まりのrank番目を指すstring_view。範囲外はnullopt。O(log(n+1))時間。
// occurrences(pattern): patternを接頭辞に持つ非空接尾辞の開始位置を接尾辞順に参照。空patternは全n本と一致。
// O((m+1)log(n+1)+1)時間・追加O(1)。mはパターン長。比較は符号なしバイト順。
// 戻り値のビューは参照先の索引を共有する個体が生きている間有効。
