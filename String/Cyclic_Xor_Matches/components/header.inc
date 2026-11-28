// SPDX-License-Identifier: CC0-1.0
// 参考（XORでの回転一致）: https://atcoder.jp/contests/abc150/tasks/abc150_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cyclic_xor_matches.md
//
// API一覧
// cyclic_xor_matches(source,target): source[(i+shift)%n]^mask==target[i]となる全組。shift昇順、O(n+1)時間・領域。
// CyclicXorMatch{shift,mask}: 回転の開始位置と全要素に共通のuint64_tのXOR値。
// 2列の長さは等しく、n<=INT_MAX/2。空なら空結果。検査有効時は異なる長さでinvalid_argument。
