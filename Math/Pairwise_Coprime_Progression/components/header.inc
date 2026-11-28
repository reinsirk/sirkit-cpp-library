// SPDX-License-Identifier: CC0-1.0
// 参考（公差を指定した等差数列の存在判定）: https://yukicoder.me/problems/no/3832
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pairwise_coprime_progression.md
//
// API一覧
// max_pairwise_coprime_progression_length(step): 正整数でどの2項も互いに素な等差数列の最大長。
// pairwise_coprime_progression_start(length, step): その長さを実現する正の初項をBigInteger<10>で返す。不能ならnullopt。
// 公差stepは正のuint64_t。step=0はinvalid_argument。空列に対しても正の初項を返す。
// 64ビット公差の最大長は105。固定個数の素数と短い整数だけを使い、時間・領域ともにO(1)。
