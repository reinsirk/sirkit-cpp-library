// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/large_convolution.md
// 参考（Verify問題）: https://judge.yosupo.jp/problem/convolution_mod_large
//
// API一覧
// convolution_mod_large<Mod=998244353,Root=3>(a,b[,transform_length]): 長さを分割し、法Modの畳み込みを返す。
// a,bはuint32_t列で、値はModで正規化。空列を含めば空列。変換長Lの既定はNTT<Mod,Root>::max_length()。
// Lは2以上の対応する2冪。短辺16以下ならO(|a||b|)、結果長がL以下なら通常のNTT畳み込み。
// 各列をP,Q個のブロックに分けるとO((P+Q)L log L+PQL)時間、O((P+Q)L)領域。
