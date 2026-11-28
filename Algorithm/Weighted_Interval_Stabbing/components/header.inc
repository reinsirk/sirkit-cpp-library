// SPDX-License-Identifier: CC0-1.0
// 参考（重み付き区間の得点）: https://atcoder.jp/contests/dp/tasks/dp_w
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/weighted_interval_stabbing.md
//
// API一覧
// StabbingInterval{left,right,weight}: 0始まりの半開区間[left,right)とint64_tの重み。負の重み・重複・空区間可。
// maximize_interval_stabbing_weight(n,intervals): 区間に選択点が1個以上あれば、その重みを1回得る。合計を最大にする。
// IntervalStabbingResult{weight,points}: 最大合計と、それを達成する0始まりの選択点の昇順列。選択なしも可。
// 0<=n<INT_MAX、0<=left<=right<=n。途中の得点と遅延加算値はint64_tに収まること。
// 区間数mとし、O(1+(n+m)*log(n+2))時間・O(n+m+1)領域。
