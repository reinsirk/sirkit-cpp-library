// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/combinatorial_sequences.md
//
// API一覧
// partition_numbers(n): 非負整数0..nを正整数の順序なし和に分ける方法の数。
// bell_numbers(n): 0..n個の要素を、空でないブロックへ分割する方法の数。
// stirling_first_row(n): 下降階乗x(x-1)...(x-n+1)のn+1係数。符号付き第1種スターリング数。
// stirling_second_row(n): n個の要素をk個の空でないブロックへ分ける方法の数。k=0..nの順。
// 以上4関数は法998244353、0≤n<2^22。第1種はO((n+1)log²(n+2))時間、ほかはO((n+1)log(n+2))時間。領域は各O(n+1)。
// bernoulli_numbers(n): B_0..B_nを返す。B_1=-1/2の規約。法998244353、0≤n<2^22。
// sum_of_powers(n,k): 1^k+...+n^kを法998244353で返す。nはuint64_t、0≤k<2^22。
// ベルヌーイ数とべき乗和は次数dについてO((d+1)log(d+2))時間・O(d+1)領域。
// derangement_numbers(n,mod=998244353): 固定点のない順列の数を、長さ0..nの順に返す。法modは正のuint32_t。
// 撹乱順列はO(n+1)時間・領域。空集合・長さ0・空積の値は1 mod mod。
// subset_sum_counts(values,max_sum): 各合計0..max_sumに対し、その和になる添字集合を法998244353で数える。
// valuesは非負。同じ値も添字で区別。m=max_sum<2^22でO(values.size()+(m+1)log(m+2))時間・O(m+1)領域。
