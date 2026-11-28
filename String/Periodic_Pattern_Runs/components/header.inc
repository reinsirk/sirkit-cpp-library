// SPDX-License-Identifier: CC0-1.0
// 参考（周期列内の反復）: https://atcoder.jp/contests/abc135/tasks/abc135_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/periodic_pattern_runs.md
//
// API一覧
// PeriodicPatternRun{start,repetitions}: 周期内の0始まりの開始位置と連続回数。無限なら回数はnullopt。
// longest_periodic_pattern_run(period,pattern): periodを繰り返す列にpatternを何個連結して置けるか求める。
// 同じ最大回数ならstart最小。1回も置けない場合は{0,0}。空patternは{0,nullopt}。
// バイト一致。長さn,mに対しO(1+n+m)時間・領域。
