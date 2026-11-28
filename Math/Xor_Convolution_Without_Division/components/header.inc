// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/xor_convolution_without_division.md
// 参考（基底変換とランク付き畳み込み）: https://info.atcoder.jp/entry/algorithm_lectures/division_free_xor_convolution
//
// API一覧
// xor_convolution_without_division(a,b): c[k]=sum_(i^j=k) a[i]*b[j]を除算せずに計算。
// 入力は同じ長さの空列または2冪。T()は加法単位元、Tには+=、-=、*=が必要。
// xor_convolution_without_division<true>(a,b): すべての元についてx+x=0の場合に積の回数を減らす。
// 符号付き整数型では計算途中の値も表現可能であること。符号なし整数型は型の幅で剰余を取る。
// 長さNに対しO(1+N log²(N+1))時間、O(N log(N+1))領域。

// SIRKIT_BUNDLED_DEPENDENCIES

#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
