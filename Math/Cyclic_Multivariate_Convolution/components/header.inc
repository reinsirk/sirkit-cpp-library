// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cyclic_multivariate_convolution.md
// 参考（問題設定）: https://judge.yosupo.jp/problem/multivariate_convolution_cyclic
// 参考（任意長変換）: https://doi.org/10.1109/TAU.1970.1162132
//
// API一覧
// cyclic_multivariate_convolution(dimensions,a,b,prime): 素数法での多次元巡回畳み込み。
// 各方向の長さがprime-1を割り切ること。最初の方向が最下位桁、全長Nは各方向の長さの積。
// a,bは長さNのuint32_t列で、入力値はprimeで正規化。N<=2^21。
// 原始根の計算を除きK方向でO(K+N log(N+1))時間、O(N)作業領域。長さ1の方向は無視する。
// dimensionsが空なら1要素同士の積を返す。
