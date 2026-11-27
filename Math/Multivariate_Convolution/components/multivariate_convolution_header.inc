// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/multivariate_convolution.md
//
// API一覧
// multivariate_convolution(dimensions,a,b): 係数列を998244353で畳み込み、各変数で次数上限を超える項を除く。
// dimensions[i]は保持する次数の数。すべて正で、積n≤2^18。1次元目が添字の下位桁。
// a,bは長さnのuint32_t列。dimensionsが空ならn=1の定数積。長さ1の軸は計算から除く。
// 幅2以上の軸数dを1以上に切り上げ、軸数kとしてO(k+d*n*(d+log(n+1)))時間・O(d*n)領域。
