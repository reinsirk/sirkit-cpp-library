// SPDX-License-Identifier: CC0-1.0
// 参考（Cantor–Zassenhaus法）: https://doi.org/10.1090/S0025-5718-1981-0606517-5
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_roots.md
//
// API一覧
// polynomial_roots(value,seed): 法998244353での根を昇順・重複なしで返す。非零定数の根は空。
// 零多項式はdomain_error。末尾ゼロを除いたN係数は2^21以下。
// 受け取った係数数をLとして期待O(L+N log³(N+1))時間、O(L)領域。seed省略時はrandom_device。
