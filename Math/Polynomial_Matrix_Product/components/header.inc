// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_matrix_product.md
// 参考（連続する引数での多項式行列積）: https://atcoder.jp/contests/arc174/editorial/9596
//
// API一覧
// polynomial_matrix_interval_product(matrix,first,count): A(first)*A(first+1)*...*A(first+count-1)を法998244353で返す。
// 正方行列の各要素は定数項順のint64_tの係数列。空は零多項式。法で正規化した最大次数は256以下。
// firstはint64_t、countはuint64_t。法の周期を使うので巨大なcountも可。count=0は単位行列、空行列は空。
// 行列の一辺をs、最大次数をd、P=998244353、H=1+sqrt((d+1)*min(count,P))とする。
// 入力係数C個でO(C+s²(d+1)²+s²H log(H+1)+s³(H+log(count+1)))時間・O(s²(d+H+1))領域。
// 形状不正はinvalid_argument、次数上限超過はlength_error。
