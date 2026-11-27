// SPDX-License-Identifier: CC0-1.0
// 参考: A. Aggarwal et al., Geometric Applications of a Matrix-Searching Algorithm, §2
// (https://doi.org/10.1007/BF01840359) の循環距離行列の補完。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/all_furthest_neighbors.md
//
// API一覧
// all_furthest_neighbors(vertices): 狭義凸多角形の各頂点から最遠の頂点の添字を入力順に返す。
// 時計回り・反時計回りともに可。添字は0始まり、同距離なら任意。
// 0点では空、1点では{0}、2点では{1,0}。各軸の座標幅は2×10^9以下。O(N)時間・領域。
