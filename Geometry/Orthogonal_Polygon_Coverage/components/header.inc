// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/orthogonal_polygon_coverage.md
//
// API一覧
// geo::OrthogonalPoint: int64_tの座標対{x,y}。geo::OrthogonalPolygon: 頂点を周回順に並べたvector。
// geo::orthogonal_polygon_coverage(polygons,cells): 各{x,y}に対し、(x+1/2,y+1/2)を含む多角形の数。
// 各多角形は単純な直交多角形。時計回り・反時計回り、異なる多角形の重なりを扱う。
// pを全頂点数、qを質問数としてO((p+q)log(p+q+2))時間、O(p+q+1)領域。
