// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/delaunay_triangulation.md
//
// API一覧
// delaunay_triangulation(points): 平面の整数点からDelaunay三角形分割の辺を返す。
// DelaunayPoint = pair<int64_t, int64_t>。DelaunayEdge = pair<size_t, size_t> は元の0始まり添字。
// 座標が同じ点は最小添字で代表。共線だけなら座標順の隣接辺、異なる座標が2点未満なら空。
// 各軸の座標幅は2×10^9以下。超えるとoverflow_error。O(N log(N+1))時間・O(N)領域。
