# 中国人郵便配達問題

`minimum_postman_route(n, edges, start, finish)` は、無向グラフの全辺を 1 回以上通る最小費用の経路を求めます。辺は `PostmanEdge{from, to, weight}` で、頂点番号は 0 始まりです。重みは非負で、自己辺と多重辺も扱えます。

`start` を省くと開始点は自動で決まり、`finish` を省くと開始点に戻ります。経路がなければ `std::nullopt` です。

戻り値の `cost` が最小費用、`vertices` が通過頂点列、`edge_ids` が通過順の入力辺番号です。

始点と終点が異なる場合は両端の次数の偶奇を反転し、奇数になる頂点をペアにします。その頂点数を `k` とすると上限は 22 です。計算量は `O(n+m + k(n+m) log(m+1) + k 2^k)` 時間・`O((k+1)n + m + 2^k)` 領域です。

[使用例](../Examples/chinese_postman_route_example.cpp) · [Verify問題](https://atcoder.jp/contests/awc0073/tasks/awc0073_e)
