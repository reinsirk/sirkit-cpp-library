// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bottleneck_pair_histogram.md
//
// API一覧
// BottleneckEdge{from,to,weight}: 0始まりの無向辺。重みは符号付き64ビット。自己辺・多重辺可。
// bottleneck_pair_histogram(n,edges): 同じ成分に属する相異なる頂点の順序なしペアを数える。
//   BottleneckPairBucket{weight,minimax_pairs,maximin_pairs} が重みの昇順に並ぶ。
//   minimax はパスの最大辺重みの最小値、maximin はパスの最小辺重みの最大値。
//   入力の各重みにつき1件。該当するペアがなければ個数0。n<=INT_MAX。
//   時間 O(n+m log(m+1))、領域 O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
