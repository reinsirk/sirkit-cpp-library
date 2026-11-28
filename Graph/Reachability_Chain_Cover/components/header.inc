// SPDX-License-Identifier: CC0-1.0
// 参考（最小チェーン被覆とフローの対応）: https://atcoder.jp/contests/ttpc2022/editorial/5121
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/reachability_chain_cover.md
//
// API一覧
// minimum_reachability_chain_cover(n,edges): 有向グラフを最少個の到達可能性チェーンに分割する。
//   0始まりの辺(from,to)を受け、チェーンごとの頂点列を返す。先の頂点から後の頂点へ到達できる。
//   到達経路は別チェーンの頂点も通れる。自己ループ・多重辺・有向閉路可。
//   強連結成分数c、縮約辺数aとしてO(n+m)の分解とO(c+a)規模の最大流1回。
//   復元はO(n+m+L)、Lは分解した縮約グラフ上の歩道の長さの合計で高々c²。領域O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
