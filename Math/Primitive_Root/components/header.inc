// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/primitive_root.md
//
// API一覧
// PrimeMultiplicativeGroup(p): 素数pを法とする乗法群。p-1をPollard–Rho法で一度だけ素因数分解する。
// modulus(): 法pを返す。
// order(a): aの位数を返す。aがpの倍数ならnullopt。O(log^2 p)回の剰余乗算。
// is_generator(a): aが原始根ならtrue。異なる素因数の数をdとしてO(d log p)回の剰余乗算。
// primitive_root(): 最小の原始根。候補をg個試すとO(g d log p)回の剰余乗算。
// primitive_root(p): 法pの乗法群を構築して最小の原始根を返す。
