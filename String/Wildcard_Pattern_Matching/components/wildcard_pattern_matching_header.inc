// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/wildcard_pattern_matching.md
//
// API一覧
// wildcard_pattern_matching(text, pattern): 各開始位置の一致をbool配列で返す。両側の*は1文字と一致。
// 入力は英小文字と*のみ。patternが長ければ空、patternが空なら全境界で一致。
// 長さは両入力とも2^19以下。範囲外はlength_error、通常版で対象外の文字はinvalid_argument。
// N=text.size(), M=pattern.size()。M<=32ならO(N+M+1)時間、他はO((N+M) log(N+M))時間。
// NTTで非負の一致判定値を厳密に計算する。追加領域はO(N+M+1)。
