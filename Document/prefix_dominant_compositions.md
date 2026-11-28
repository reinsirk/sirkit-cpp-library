# 接頭辞で比率を保つ2つの数列の数え上げ

`prefix_dominant_composition_counts<Mod>(max_length, p, q)` は、長さごとに非負整数列 `A, B` の組数を返します。全体の和がそれぞれ `p, q` で、どの接頭辞でも $q\sum A\ge p\sum B$ を満たす組を数えます。

- `p, q` は正の互いに素な `uint64_t` です。法 `Mod` は素数で、既定値は $998244353$ です。
- 結果の添字は長さを表し、0番目は0です。`max_length < Mod` が必要です。
- 構築は $O(\text{max_length}+1)$ 時間・領域です。

たとえば `p=q=1` のとき、`A` の1が `B` の1より先か同じ位置にあればよいので、長さ1、2、3の答えは順に1、3、6です。

[使用例](../Examples/prefix_dominant_compositions_example.cpp) ／ [Verify](../Verify/current/submissions/prefix_dominant_compositions_yuki3480.cpp) ／ [Cycle Lemma](https://info.atcoder.jp/entry/algorithm_lectures/cycle_lemma)
