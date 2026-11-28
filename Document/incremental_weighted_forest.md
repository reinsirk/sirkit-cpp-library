# 辺追加だけの重み付き森

異なる木を辺でつなぎながら、2頂点間の距離と木の直径を求めます。距離は経路に含まれる辺の重みの和です。[本体](../Graph/Incremental_Weighted_Forest.cpp)

## `IncrementalWeightedForest`

```text
explicit IncrementalWeightedForest(int n);
```

頂点番号が $0$ 始まりの $n$ 頂点をすべて孤立させて始めます。準備に $O(n\log(n+1))$ 時間・領域を使います。

## `link`

```text
bool link(int u, int v, std::uint64_t weight);
```

別々の木に属する `u` と `v` を無向辺で結び、`true` を返します。同じ木なら変更せず `false` です。1本の木の辺の重み合計は `std::uint64_t` に収めてください。小さい方の木の頂点数を $k$ とすると、1回の追加は $O(k\log(n+1))$ 時間で、成功した追加すべてで $O(n\log^2(n+1))$ 時間です。

## 距離・直径・連結性

```text
std::optional<std::uint64_t> distance(int u, int v) const;
Diameter diameter(int v) const;
bool connected(int u, int v) const;
int component_size(int v) const;
std::size_t size() const noexcept;
```

`distance` は距離を返し、別の木どうしなら `std::nullopt` です。`diameter` は `v` が属する木について、端点と長さを `Diameter{first, second, distance}` で返します。孤立頂点の直径は $0$ です。

`connected` は同じ木か、`component_size` は指定した木の頂点数を返します。照会は $O(\log(n+1))$ 時間で、全頂点数を返す `size` だけ $O(1)$ 時間です。

## 使用例

```cpp
sirkit::IncrementalWeightedForest forest(4);
forest.link(0, 1, 3);
forest.link(1, 2, 4);
const auto distance = forest.distance(0, 2);  // 7
const auto diam = forest.diameter(1);         // 端点0,2、長さ7
```

[使用例](../Examples/incremental_weighted_forest_example.cpp)・[yukicoder Verify](../Verify/current/submissions/incremental_weighted_forest.cpp)
