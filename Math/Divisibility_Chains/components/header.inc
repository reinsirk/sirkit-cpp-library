// SPDX-License-Identifier: CC0-1.0
// 参考（数列の条件とVerify問題）: https://atcoder.jp/contests/arc116/tasks/arc116_c
// https://yukicoder.me/problems/no/2798
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/divisibility_chains.md
//
// API一覧
// divisibility_chain_end_counts(length, max_value): a[i]がa[i+1]を割り、最後がxの正整数列の個数をresult[x]に返す。
// max_valueは非負のint。IntegerSieveを渡すとその範囲の最小素因数表を再利用する。result[0]=0、法998244353。
// lengthは正のuint64_t。時間・追加領域O(1+max_value)。
// divisibility_chain_product_count(value): 初項2以上、各項が次を割り、全項の積がvalueの数列数を正確なuint64_tで返す。
// 長さは任意の正整数。valueは正のuint64_t、value=1では0。素因数分解の期待時間に加え素因数の種類数に比例。
