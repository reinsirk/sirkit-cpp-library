// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/circle_tangents.md
//
// API一覧
// geo::CircleTangent{normal, first, second}: 単位法線と両円の接点。直線はnormal·(x-first)=0。
// geo::point_circle_tangents(circle, point, eps={}): 点を通る接線と円上の接点を0〜2本返す。secondは指定点。
// geo::circle_tangents(first, second, eps={}): 内・外共通接線を最大4本返す。同心同半径ならnullopt。
// 半径は正。各APIはO(1)時間・領域。epsが許容する距離差の範囲では接する場合として扱う。
