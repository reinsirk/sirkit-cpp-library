# 木の距離分布

距離ごとに頂点対の個数を数えます。非重みの無向木を、辺を両向きに入れた隣接リストで渡します。距離は頂点間の経路に含まれる辺の本数です。`Graph` は `std::vector<std::vector<int>>` です。

```cpp
const sirkit::Graph path{{1}, {0, 2}, {1}};
const auto count = sirkit::tree_distance_frequency(path);  // {0, 2, 1}
```

## `tree_distance_frequency`

```text
std::vector<std::int64_t> tree_distance_frequency(const Graph &graph);
```

`count[d]` は距離が `d` の異なる頂点の非順序対の個数です。戻り値は長さ `n` の `std::vector<std::int64_t>` で、頂点があれば `count[0]` は0です。空の入力では空の配列を返します。

頂点数の上限は $2^{23}$ です。重心を通る頂点対を枝の距離分布の畳み込みで数え、整数の個数を正確に求めます。$O(n\log^2(n+1))$ 時間・$O(n)$ 領域です。

`CentroidDecomposition` と `NTT` の公開 API を同梱しています。 [重心分解](centroid_decomposition.md) ／ [NTT](ntt.md)

[使用例](../Examples/tree_distance_frequency_example.cpp) ／ [Verify](../Verify/current/submissions/frequency_table_of_tree_distance.cpp)
