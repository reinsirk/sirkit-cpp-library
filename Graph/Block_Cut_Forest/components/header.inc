// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/block_cut_forest.md
//
// API一覧
// block_cut_forest(n,edges): 元の頂点と点二重連結ブロックを頂点とする無向の森を返す。
// 元の頂点番号は[0,n)、ブロックはn以上。ブロックへの隣接頂点が所属メンバー。
// 孤立点はそのまま、自己辺は1辺ごとのブロックとする。平行辺は同じブロックにまとまる。
// ブロックの番号・隣接順は規定しない。O(n+m)時間・領域、mは元の辺数。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
