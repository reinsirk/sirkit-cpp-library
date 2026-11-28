// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/integer_geometry_3d.md
//
// API一覧
// Point3i{x,y,z}: int64_tの3次元整数点。各座標の絶対値は10^9以下。
// orientation_3d(a,b,c,p): (b-a)×(c-a)と(p-a)の内積の符号。正1、負-1、零0。整数演算で厳密。
// triangular_prism_position(a,b,c,p): pの平面への垂直射影を三角形と比較。内部・境界・外部・退化。
// signed_plane_distance(a,b,c,p): 法線方向の符号付き距離をdoubleで返す。3点が共線ならnullopt。
// 判定は整数で厳密。符号付き距離は法線の長さと除算に浮動小数点を使う。各操作はO(1)時間・領域。
