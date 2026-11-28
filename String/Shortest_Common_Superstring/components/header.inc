// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_common_superstring.md
//
// API一覧
// shortest_common_superstring(words): 各要素を連続部分文字列として含む最短の文字列を一つ返す。
// 空の要素、重複、他の要素に含まれる要素を除く。入力20個以下、全長の合計はUINT32_MAX以下。
// Nは入力数、Sは合計長、Kは除外後の要素数。O(N*S+K^2*2^K+1)時間・O(S+K*2^K+N^2+1)領域。
