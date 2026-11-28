// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/stirling_columns.md
//
// API一覧
// stirling_first_column(n,k): 符号付き第1種スターリング数s(k,k)..s(n,k)を返す。
// stirling_second_column(n,k): 第2種スターリング数S(k,k)..S(n,k)を返す。
// 法998244353、n<法、L=n-k+1は2^22以下。k>nなら空。
// 各O(L log(L+1)+log(k+1))時間、O(L)領域。
