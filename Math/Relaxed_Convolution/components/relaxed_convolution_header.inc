// SPDX-License-Identifier: CC0-1.0
// 参考（オンライン畳み込みの分割）: https://atcoder.jp/contests/abc315/editorial/6988
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/relaxed_convolution.md
//
// API一覧
// [RelaxedConvolution<Mod=998244353,Root=3>]
// push(a,b): 次の添字iにa,bを追加してΣ_{j=0}^i A[j]*B[i-j] mod Modを返す。係数はuint32_t。
// size(): 入力済みの組数。max_size(): 使用するNTTの最大変換長。各O(1)。
// clear(): 入力済みの組を消して、容量を再利用する。Mod/Rootの条件はNTTと同じ。
// n回のpushでO(n log²(n+1))時間・O(n)領域。上限を超えるpushはlength_error。
