// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/krawtchouk_transform.md
// 参考（遷移行列の固有関数）: https://info.atcoder.jp/entry/algorithm_lectures/eigenfunction
// 参考（Krawtchouk行列）: https://arxiv.org/abs/quant-ph/0702073
//
// API一覧
// krawtchouk_transform(f): 入力長d=N+1で、∑ f[i](1-x)^i(1+x)^(N-i)の係数列を返す。
// inverse_krawtchouk_transform(f): 同じ長さのKrawtchouk変換を元に戻す。
// 法998244353、長さdの上限2^22。空列なら空。末尾0も含めてNを決める。
// O(1+d log(d+1))時間、O(d)領域。表を作らずTaylor Shiftを2回使う。

// SIRKIT_BUNDLED_DEPENDENCIES

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
