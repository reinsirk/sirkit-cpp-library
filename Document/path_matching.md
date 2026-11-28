# パス上の重み付きマッチング

辺 `(i,i+1)` の重みを `weights[i]` に入れます。互いに接しない辺を選ぶので、隣り合わない要素をちょうど `k` 個選ぶ問題にも使えます。例えば重みが `[3, 5, 3]` なら、1 本の最適値は 5、2 本では両端を選んで 6 です。

- `PathMatching<T>(weights)` は全本数の最適値を前処理します。`best(k)` で値、`edge_indices(k)` で辺番号の一例を取得できます。`values()` の添字は本数で、`values()[0]` は 0 です。
- 値が 1 本数分だけ必要なら `maximum_path_matching_weight<T>(weights, k)` を使えます。

負の重みも使えます。重みの交代和と最適値を表せる型 `T` を指定してください。前処理は `O((m+1) log(m+2))` 時間、値の照会は `O(1)`、解の復元と単独の値の計算はそれぞれ `O(m+1)` 時間と平均 `O(m+1)` 時間です。領域は `O(m+1)` です。

[使用例](../Examples/path_matching_example.cpp) · [飴](https://atcoder.jp/contests/joisc2018/tasks/joisc2018_j) · [パス上のマッチングとその応用](https://info.atcoder.jp/entry/algorithm_lectures/matching_on_path)
