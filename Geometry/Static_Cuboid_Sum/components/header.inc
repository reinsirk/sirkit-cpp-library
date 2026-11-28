// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_cuboid_sum.md
//
// API一覧
// StaticCuboidSumPoint{x,y,z,weight}: 3次元の重み付き点。座標・重みは符号付き64bit。同じ座標の点も加算する。
// StaticCuboidSumQuery{lower,upper}: 配列{x,y,z}で指定した直方体。各軸でlowerを含みupperを含まない。
// static_cuboid_sum(points,queries): 各直方体内の重みの総和を照会順に返す。空直方体は0。
// 重みの総和は2^64を法とし、符号付き64bitの値として返す。検査有効時は逆転範囲でinvalid_argument。
// 点N個・照会Q個について O((N+Q)log²(N+Q+2))時間、O(N+Q)追加領域。
