// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/union_find_order_statistics.md
// 同梱UnionFindの出典: https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/union-find
//
// API一覧
// UnionFindOrderStatistics<T>(labels): 初期ラベルを固定し、全頂点を独立成分にする。頂点は0始まり。
// Tはコピー可能な全順序型。異なる値は座標圧縮し、同じ値は多重度に数える。
// merge(a,b): 新規結合ならtrue。全結合での木の併合時間はO(n log(m+1))。成分探索は償却O(α(n))。
// root(v)/issame(a,b)/size(v): 代表/同一成分/成分サイズ。償却O(α(n))。
// size()/component_count(): 全頂点数/成分数。O(1)。
// kth(v,k): 成分内のラベルの昇順で0始まりk番目。範囲外はnullopt。
// count(v,label)/count_less(v,label): 成分内で同値/未満のラベル数。
// 照会はTの比較とコピーをO(1)としてO(log(m+1)+α(n))償却。
// nは頂点数でINT_MAX以下、mは異なるラベルの個数。構築O(n log(n+1))時間・O(n log(m+1))領域。
