# 強連結トーナメントの個数と終端成分の分布

`tournament_strong_counts(n)` は、0 から `n` 頂点までのラベル付き強連結トーナメントの個数を返します。戻り値の添字が頂点数です。3 頂点では有向三角形が 2 通りあります。

`tournament_sink_size_probabilities(n)` は、すべてのトーナメントを等確率で選んだときの終端成分の大きさを集計します。終端成分は、外へ出る辺がない強連結成分で、空でないトーナメントには 1 つだけあります。戻り値の `p[k]` が大きさ `k` の確率です。

法は `998244353` で、確率は分母の逆元を掛けた値です。頂点がない場合、空のトーナメントは強連結の個数に含めず、分布は `p[0] = 1` です。頂点数が `2^22` 未満の入力を扱います。どちらの操作も `O((n+1) log(n+2))` 時間・`O(n+1)` 領域で計算できます。

[使用例](../Examples/tournament_strong_counts_example.cpp) · [Verify問題](https://atcoder.jp/contests/pakencamp-2025-day1/tasks/pakencamp_2025_day1_p) · [トーナメントの解説](https://info.atcoder.jp/entry/algorithm_lectures/tournament)
