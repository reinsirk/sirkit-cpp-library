# 有向辺を追加して強連結にする

`strong_connectivity_augmentation(n, edges)` は、どの頂点からもほかの頂点に到達できるようにする最小本数の追加辺を返します。追加辺はすべて入力にない辺です。

入力と出力は `(from, to)` の組で、頂点番号は 0 始まりです。自己辺、多重辺、孤立頂点を含むグラフも扱えます。

強連結成分を縮約して、入次数 0 の成分と出次数 0 の成分をつなぎます。必要な本数は入次数 0 の成分数と出次数 0 の成分数のうち、大きい方です。空グラフやすでに強連結なグラフには辺を追加しません。

計算量は `O(n+m)` 時間・領域です。

[使用例](../Examples/strong_connectivity_augmentation_example.cpp) · [Verify問題](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=0366)
