# 無向グラフの大域最小カット

全頂点を二つの空でない集合に分け、両方の集合をまたぐ辺の重みの和を最小にします。

```cpp
const std::vector<sirkit::GlobalMinCutEdge> edges{{0, 1, 7}, {1, 2, 2}};
const auto cut = sirkit::global_min_cut(3, edges);  // 重み2、片側は頂点2
```

## `global_min_cut`

```text
std::optional<GlobalMinCutResult> global_min_cut(int n,
                                                std::span<const GlobalMinCutEdge> edges);
```

`GlobalMinCutEdge{u, v, weight}` で無向辺を渡します。頂点番号は `[0,n)` です。重みは `std::uint64_t` で、省略すると1です。平行辺は重みを足し、自己辺は無視します。自己辺を除いた全辺の重みの合計は `std::uint64_t` に収めます。

戻り値の `weight` は最小の重み、`side` は片側の頂点一覧です。頂点が2個未満なら `std::nullopt` を返します。$n$ 頂点 $m$ 辺で $O(n^3+m)$ 時間・$O(n^2)$ 領域です。

[使用例](../Examples/global_min_cut_example.cpp) ／ [参照テスト](../Verify/current/tests/global_min_cut_reference_test.cpp) ／ [原論文](https://doi.org/10.1145/263867.263872)
