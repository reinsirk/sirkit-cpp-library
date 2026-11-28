// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/successor_set.md
// 参考（Parking Function）: https://info.atcoder.jp/entry/algorithm_lectures/parking_function
//
// API一覧
// SuccessorSet(n=0): 0..n-1が残る集合を作る。0<=n<INT_MAX。O(n+1)時間・領域。
// size()/remaining(): 最初の要素数/残っている要素数。O(1)。
// next(i): i以上の最小要素。該当なしならnullopt。0<=i<=n。
// erase(i): iを取り除く。存在したときtrue。take(i): next(i)を返し、見つかれば取り除く。
// take_cyclic(i): i以上がなければ先頭から探すtake。next/erase/take/take_cyclicは償却O(α(n+1))。
// parking_slots(preferences): 各希望位置から右に最初の空きへ順に配置する。失敗はnullopt。償却O(n α(n+1))時間・O(n)領域。
