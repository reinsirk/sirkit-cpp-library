// SPDX-License-Identifier: CC0-1.0
// 数学的参考: J. L. Massey, Shift-Register Synthesis and BCH Decoding (1969).
// https://www.isiweb.ee.ethz.ch/archive/massey_pub/pdf/BI411.pdf
// 数学的参考（講座）: https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_reconstruction
// 参考（用途・Verify問題）: https://nyaannyaan.github.io/library/fps/berlekamp-massey.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/linear_recurrence.md
//
// API一覧
// BerlekampMassey<T>(s): 体T上の有限列sに合う最短線形漸化式の係数cを返す。O(n²)時間・O(n)領域。
// 返却長dとし、d<=i<nでs[i]=sum(c[j]*s[i-j-1])。同じ最短次数の解は一意とは限らない。
// Tは正確な体の四則演算・0/1からの構築・等値比較を提供する。Mintは利用側が用意する。
// BerlekampMassey(vector<uint32_t>): 法998244353で同じ係数を返す。入力は自動で正規化する。
// 空列・全零列は{}。有限列の適合だけを保証し、未知の後続項や本来の漸化式の特定は保証しない。
