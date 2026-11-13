// SPDX-License-Identifier: CC0-1.0
// 参考（数式・Verify問題。CRT復元・値域管理は独自実装）:
// https://info.atcoder.jp/entry/algorithm_lectures/chinese_remainder_theorem
// https://judge.yosupo.jp/problem/convolution_mod_1000000007
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/convolution.md
//
// API一覧
// CRTConvolution: 3素数NTTを所有する畳み込み。コピーの根cacheは独立、移動可。Mint不要。
// modular(a,b,mod): 整数vectorを1<=mod<=UINT32_MAXへ正規化し、modでの畳み込みvector<uint32_t>を返す。
// modular_square(a,mod): 同列の畳み込み。大規模では各NTTの順変換を1回にする。
// integer(a,b)/integer_square(a): int64_t列の厳密な畳み込みvector<int64_t>。全ての最終係数がint64_t内であることが前提。
// 中間の整数係数が大きくても相殺を扱う。前提違反を全て検出する保証はない。入力は変更しない。空を含めば{}。
// 結果長はmax_length()=2^23以下。短辺32以下ではO(nm)、それ以外はL=bit_ceil(n+m-1)としてO(L log L)。追加O(L)。
// prepare(length): 0または上限以下の2冪長の根を準備。新規O(length)、保持O(length)。長さ/容量の制限は検査省略時も維持。
// roots_capacity()/clear_cache(): 3表の保持根数の合計を返す/根を解放する。O(1)/O(保持数)。
// ConvolutionAnyMod(a,b,mod)/ConvolutionInt64(a,b): 一時オブジェクトを使う簡易版。反復時はCRTConvolutionを再利用する。
// GCC/Clangの128ビット整数拡張があれば厳密復元に使用。SIRKIT_CONVOLUTION_DISABLE_INT128または
// SIRKIT_FORCE_STANDARD_ARITHMETICで標準2語復元を選べる。
