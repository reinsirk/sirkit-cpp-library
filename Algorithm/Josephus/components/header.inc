// SPDX-License-Identifier: CC0-1.0
// 参考（円環の除去規則とVerify問題）: https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=0085
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/josephus.md
//
// API一覧
// josephus_survivor(n, step): 番号0を1番目として数え、step番目を除く。直後から再開し、最後に残る番号を返す。n,stepは正のuint64_t。
//   stepが1か2ならO(1)。一般にO(min(n,step log(n+1)))時間。領域は高速化した漸化式でO(step log(n+1))、通常の漸化式でO(1)。
// josephus_order(n, step): 同じ規則の除去順をvector<int>で返し、末尾が最後の番号。nは非負のint、n=0なら空。
//   O(1+n log(n+1))時間、O(1+n)領域。番号はどちらも0始まり。step=0はinvalid_argument。
