// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_differential_operator.md
// 参考（微分を含む遷移）: https://info.atcoder.jp/entry/algorithm_lectures/eigenfunction
//
// API一覧
// polynomial_differential_operator_power(f,p,a,t,n): g↦p(x)g′(x)+a(x)g(x)をfへt回適用し、先頭n係数を返す。
// 法998244353。t>0かつn>0ならp(0)≠0。N=t+n<=2^18。n=0なら空、t=0ならfの先頭n係数。
// O(N log²(N+1))時間、O(N log(N+1))領域。f,p,aのx^N以上の項は不要。

// SIRKIT_BUNDLED_DEPENDENCIES

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>
