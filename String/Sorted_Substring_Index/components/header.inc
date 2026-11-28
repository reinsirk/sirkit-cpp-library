// SPDX-License-Identifier: CC0-1.0
// 参考（並べ替え後の連続部分の条件）: https://atcoder.jp/contests/abc285/tasks/abc285_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sorted_substring_index.md
//
// API一覧
// SortedSubstringIndex<Alphabet=26,First='a'>(text): 固定長の文字列を保持。O(n+Alphabet)時間・領域。
// 使うバイトは[First,First+Alphabet)。n<=INT_MAX。空構築可。
// size()/empty()/get(i): 長さ/空か/位置iのバイト。O(1)。set(i,byte): 一点置換。O(log(n+1))。
// is_nondecreasing(l,r): 現在の半開区間[l,r)が昇順か。O(log(n+1))。
// is_sorted_slice(l,r): 全体を昇順に並べた結果に、元の[l,r)が連続部分として現れるか。O(log(n+1)+Alphabet)。
// 符号なしバイト順、添字は0始まり、空区間はtrue。コピーは独立、移動元は空。
