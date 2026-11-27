// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/concave_min_plus_convolution.md
//
// API一覧
// min_plus_convolution_concave_arbitrary(a,b): kごとにmin(a[i]+b[j]) (i+j=k)を返す。
// aは隣接差分が非増加の凹数列、bは任意。int64_tで和が表現可能であること。片方が空なら空。
// n=a.size()、m=b.size()。結果の長さはINT_MAX以下。O((n+m)log(n+m+1))時間・O(n+m)領域。
