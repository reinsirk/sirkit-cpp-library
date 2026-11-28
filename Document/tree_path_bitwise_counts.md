# 木のパスの AND・OR・XOR を集計する

重み付きの木や森で、各頂点ペアのパスに沿って辺の重みを AND、OR、XOR します。ビットごとに、その演算結果が 1 になるペアの数を求めます。

`tree_path_bitwise_counts(n, edges)` に頂点数と、`TreePathBitwiseEdge{from, to, weight}` の列を渡します。頂点は 0 始まりで、重みは `uint64_t` です。同じ連結成分内の、相異なる頂点の順序なしペアを数えます。

返り値の `and_pairs[b]`、`or_pairs[b]`、`xor_pairs[b]` は各演算のビット `b` の個数です。総和が必要なら、個数に `2^b` を掛けて足します。剰余で答えるときも同じ式を使えます。

辺数を m、辺重みに現れるビットの種類数を B とすると、時間は `O((B + 1)(n + m))`、領域は `O(n + m)` です。

[使用例](../Examples/tree_path_bitwise_counts_example.cpp) · [AND の Verify](https://yukicoder.me/problems/no/1843) · [XOR の Verify](https://atcoder.jp/contests/abc201/tasks/abc201_e)
