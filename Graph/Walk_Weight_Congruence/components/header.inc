// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/walk_weight_congruence.md
//
// API一覧
// WalkCongruenceEdge{from,to,weight}: 0始まりの有向辺と符号付き int32_t 重み。
// WalkWeightCongruence(n,edges): 強連結成分ごとに閉歩道の重みの最大公約数を求める。
//   構築 O(n+m log(nW+2)) 時間・O(n+m) 追加領域。W は重みの絶対値の上限。
// period(v)/periods(): vの成分の周期/全成分の周期の範囲。すべての閉歩道の重みが0なら周期0。O(1)。
// relation(u,v): 同成分なら WalkWeightClass{offset,period}。別成分はnullopt。O(1)。
//   重みは offset modulo period。period=0ならすべて同じ offset。頂点と辺の再訪を許す。
// WalkWeightClass::matches(residue,modulus): 指定余りの歩道が存在するか。modulus>0。O(log(modulus+1))。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
