// SPDX-License-Identifier: CC0-1.0
// 参考（Kinoshita–Liの合成）: https://arxiv.org/abs/2404.05177
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fps_composition.md
//
// API一覧
// fps_compose(outer,inner,precision): outer(inner(x))をx^precision未満で切る。innerの定数項は0。
// fps_compositional_inverse(value,precision): value(g(x))=x mod x^precisionを満たすg。
// 逆合成ではvalueの定数項0・一次係数非零。精度0なら空。精度Nは2^18以下、法998244353。
// 合成と逆合成は各O(N log²(N+1))時間、O(N log(N+1))領域。
// fps_power_projection(value,weight,degree,count): k=0..count-1の[x^degree]weight(x)*value(x)^k。
// degree<2^18、count<=2^18。D=degree+1、C=countとしてO(D log²(D+1)+C log(C+1))時間、O(D+C)領域。
