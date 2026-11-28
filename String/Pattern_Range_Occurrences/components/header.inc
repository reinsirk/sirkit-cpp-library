// SPDX-License-Identifier: CC0-1.0
// 参考（区間内の出現）: https://atcoder.jp/contests/abc477/tasks/abc477_c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pattern_range_occurrences.md
//
// API一覧
// PatternRangeOccurrences(text,pattern): 固定パターンの出現を前計算。長さn,mに対しO(1+n+m)時間・領域。
// size()/pattern_size() const: 文字列とパターンの長さ。O(1)。
// count(l,r) const: 半開区間[l,r)全体に収まる出現数。O(1)。
// positions(l,r) const: 同じ出現の開始位置を昇順の借用spanで返す。O(1)。重なりも数える。
// 0始まり・バイト比較。空パターンは各境界l..rに出現。コピーは内部の読み取り専用の表を共有する。
// 既定構築・move元は両方が空の状態。借用spanは表を所有する実体が残る間有効。
