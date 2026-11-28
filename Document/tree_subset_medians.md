# 木の部分集合に対する中央値の個数

木の頂点から $k$ 個選び、各頂点から選んだ頂点への距離を足します。その距離和が最小になる頂点を中央値と呼びます。

- `TreeSubsetMedians(n, edges, eligible={})` は0始まりの木を前処理します。`eligible` を渡すと、値が0でない頂点だけを選べます。省略すると全頂点が候補になります。構築は時間・領域ともに $O(1+n)$ です。
- `median_count_sum(k, choose)` は、すべての $k$ 頂点の選び方について中央値の個数を足します。`choose(n,k)` には組合せ数を求める関数を渡します。
- `cut_histogram()[s]` は、辺を切ったとき少ない側に選べる頂点が $s$ 個ある辺の数です。`eligible_count()` は候補数を返します。

候補数を $e$ とすると、期待値は集計値を `choose(e,k)` で割った値です。法を使う場合は、分母の逆元が存在すればその逆元を掛けます。`choose` が $O(1)$ なら集計は偶数 $k$ で $O(1+e-k)$、奇数 $k$ で $O(1)$ です。

[使用例](../Examples/tree_subset_medians_example.cpp) ／ [Verify](../Verify/current/submissions/tree_subset_medians_yuki3834.cpp)
