# 最短の奇数長閉路

`shortest_odd_cycle(n, edges)` は無向グラフで辺の本数が奇数の単純閉路のうち、最も短いものを返します。

頂点番号を 0 始まりとし、無向辺を `pair<int, int>` で渡します。戻り値は閉路を回る順に並べた頂点列で、始点は末尾に重ねません。奇数長の閉路がなければ空列です。自己辺は 1 頂点の閉路になり、多重辺も使えます。

歩数の偶奇を分けて BFS を行います。二部グラフの連結成分は探索を省き、全体の時間は最悪 `O(n(n+m))`、領域は `O(n+m)` です。

[使用例](../Examples/shortest_odd_cycle_example.cpp) · [Verify問題](https://yukicoder.me/problems/no/3291)
