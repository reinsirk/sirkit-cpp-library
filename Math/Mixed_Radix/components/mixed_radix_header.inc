// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/mixed_radix.md
//
// API一覧
// [MixedRadix<Radix=10>]
// MixedRadix(radices): 正のuint32_tを上位桁から渡す。1も可。桁数n、全基数の積の桁数D。
// capacity(): 表現可能な整数の個数、基数の積のconst参照。O(1)。
// encode(digits): 同じ長さ・0<=digits[i]<radices[i]の桁をBigIntegerへ。decode(value): 桁へ戻す。
// encodeと構築はO(n + D log(D+1)log(n+1))時間。decodeはO(n + D log²(D+1)log(n+1))。
// 積を保持する領域はO(n+D log(n+1))。整数の範囲は0..capacity()-1。RadixはBigIntegerの基数。
