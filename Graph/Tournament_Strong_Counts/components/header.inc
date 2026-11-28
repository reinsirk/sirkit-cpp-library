// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tournament_strong_counts.md
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/tournament
//
// API一覧
// tournament_strong_counts(n): k=0..n の各 k について、ラベル付き強連結トーナメントの個数。
//   添字0は0、添字1は1。法998244353。
// tournament_sink_size_probabilities(n): 一様ランダムな n 頂点トーナメントの終端強連結成分の大きさの分布。
//   戻り値[k]は大きさkの確率。法998244353で分母の逆元を掛けた値。n=0 は {1}。
//   0<=n<2^22。どちらも O((n+1) log(n+2)) 時間、O(n+1) 領域。
