# 森を同じ大きさのパスに分ける

森の各頂点をちょうど一度ずつ使い、頂点数が同じパスに分けます。分解できれば、各パスを端からたどる順に頂点列で返します。

`partition_forest_into_equal_paths(n, edges, path_size)` に頂点数、0 始まりの無向辺、パス一つに含める頂点数を渡します。パスの頂点数は 1 以上です。森の全頂点を分解できなければ `nullopt` が返ります。

```cpp
const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}, {2, 3}};
auto paths = sirkit::partition_forest_into_equal_paths(4, edges, 2);
assert(paths && paths->size() == 2);
```

頂点数を n、辺数を m とすると、時間・領域はともに `O(n + m)` です。各成分の葉から順に、未完成のパスを親へ引き継ぐか、そこで完成させるかを決めます。

[使用例](../Examples/equal_tree_path_partition_example.cpp) · [Verify](https://atcoder.jp/contests/abc397/tasks/abc397_e)
