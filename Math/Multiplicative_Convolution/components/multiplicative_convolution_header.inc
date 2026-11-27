// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/multiplicative_convolution.md
//
// API一覧
// multiplicative_convolution_prime(a,b): 長さが素数pの係数列。a[i]*b[j]を添字i*j mod pに足す。
// 2≤p≤2^19。法pの添字0への和も返す。O(p log(p+1)+g log²(p+1))時間・O(p)領域。gは原始根候補の試行数。
// multiplicative_convolution_power_of_two(a,b): 長さが2冪nの係数列。添字i*j mod nに同様に足す。
// n=2^e、0≤e≤20。O(n log(n+1))時間・O(n)領域。
// 両方ともa,bは同じ長さのuint32_t列。係数の計算は法998244353。
