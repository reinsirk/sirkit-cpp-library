// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_sequence_product.md
//
// API一覧
// polynomial_sequence_product(factors): 法998244353で多項式の列の積を求める。
// 係数列は定数項から。末尾の0は除き、零多項式は空、空積は{1}。値渡しにstd::moveで領域を渡せる。
// Dは零を除く次数の合計でD+1<=2^23。短い順にNTTで積を取る。全入力係数Cに対し
// O(C+D log²(D+2))時間・O(C+1)領域。定数因子はスカラーにまとめる。
