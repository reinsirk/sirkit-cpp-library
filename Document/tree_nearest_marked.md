# 木上の最近接マーク頂点

木の各頂点のマークを切り替えながら、指定した頂点から最も近いマーク済み頂点を求めます。距離は経路の辺数です。[本体](../Graph/Tree_Nearest_Marked.cpp)

## `TreeNearestMarked`

```text
explicit TreeNearestMarked(const Graph &graph);
```

`graph` は $0$ 始まりの頂点番号を持つ単純無向木の隣接リストです。空の木も渡せます。構築直後はすべて未マーク。重心分解で前処理し、頂点数 $n$ に対し $O(n\log(n+1))$ 時間・領域です。

## マークの更新・参照

```text
void set_marked(int v, bool marked = true);
bool toggle(int v);
bool marked(int v) const;
std::size_t size() const noexcept;
```

`set_marked` は `v` のマークを設定または解除します。`toggle` はマークを反転し、反転後の状態を返します。いずれも $O(\log^2(n+1))$ 時間です。`marked` と `size` はマークの状態と頂点数をそれぞれ $O(1)$ で返します。

## `nearest`

```text
std::optional<Result> nearest(int v) const;
```

`Result{vertex, distance}` に最も近いマーク済み頂点の番号と距離が入ります。同じ距離なら頂点番号が最小のものを返し、マーク済み頂点がなければ `std::nullopt` です。$O(\log(n+1))$ 時間です。

## 使用例

```cpp
sirkit::TreeNearestMarked tree({{1}, {0, 2}, {1}});
tree.set_marked(2);
const auto nearest = tree.nearest(0); // vertex=2、distance=2
tree.toggle(2);                       // マークを解除
```

[使用例](../Examples/tree_nearest_marked_example.cpp)・[yukicoder Verify](../Verify/current/submissions/tree_nearest_marked.cpp)
