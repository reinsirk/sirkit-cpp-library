# 群ラベル付きの非零最短パス

無向グラフの各辺に長さと群ラベルがあるとき、ラベルの積が単位元以外になる最短の単純パスを、1 頂点から各頂点へ求めます。例えばラベルを XOR で合成し、パスの XOR が 0 以外になる最短長を求められます。

`shortest_nonzero_paths(n, edges, source, identity, combine, inverse)` を呼びます。頂点は 0 始まりです。辺は `GroupPathEdge<Label>{from, to, length, label}` で、`length` は `uint32_t` です。

- `combine(a, b)` は通過順の積、`inverse(a)` は逆元を返します。ラベルの等値比較には `==` を使います。
- 辺を逆向きに通るときは、ラベルの逆元を使います。同じ頂点を再訪する経路は対象外です。
- 結果は頂点順の `vector<optional<uint64_t>>` です。該当するパスのない頂点と始点には `nullopt` が入ります。

群演算と等値比較が `O(1)` なら、時間は `O(n + m log(m + 2))`、領域は `O(n + m)` です。

[使用例](../Examples/shortest_nonzero_paths_example.cpp) · [Verify問題](https://yukicoder.me/problems/no/1602) · [Iwata・Yamaguchi の論文](https://arxiv.org/abs/1906.04062)
