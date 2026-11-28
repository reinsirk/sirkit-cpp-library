// SPDX-License-Identifier: CC0-1.0
// 参考（状態DPと漸化式復元）: https://atcoder.jp/contests/abc305/editorial/6552
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pattern_avoidance.md
//
// API一覧
// count_strings_avoiding_patterns<Alphabet=26,First='a',Field=uint32_t>(length,forbidden):
// 指定されたバイト範囲の長さlengthの列で、禁止語を連続部分に持たないものの個数。
// uint32_tは法998244353、別のFieldは正確な体で計算。空の禁止語を含むと0。lengthはuint64_t。
// 禁止語の合計長をP、利用可能状態をS、A=Alphabet、t=min(length+1,2S)とする。
// Aho–CorasickとDPでO(A*(P+1)+A*S*t)時間。長い場合はO(S²)のBerlekamp–Masseyと第length項計算を加える。
// 漸化式の次数d、L=bit_ceil(2d+1)として、第length項は既定型でO(L logL log(length+2))、別の体でO(d² log(length+2))。
// 補助領域O(A*(P+1))。同じ禁止語の重複は結果に影響しない。
