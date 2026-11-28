# スターの辺を更新する全域最小カット

無向グラフに頂点を1個加え、元の全頂点と辺で結びます。追加した辺の重みを変更するたびに、全域最小カットの重みを求めます。[本体](../Graph/Dynamic_Star_Min_Cut.cpp)

## `DynamicStarMinCut`

```text
DynamicStarMinCut(int n, std::span<const DynamicStarCutEdge> edges,
                  std::span<const std::uint64_t> star = {});
```

元の頂点は `[0,n)`、追加する頂点は `n`。元の辺を `DynamicStarCutEdge{u, v, weight}` で渡します。平行辺は重みを合算し、自己辺は無視します。`star[v]` は `v` と `n` を結ぶ辺の重みで、省略時はすべて $0$ です。

$n \geq 1$ で、元の辺と追加した辺の重みの合計を `std::uint64_t` に収めます。元の辺数 $m$ に対し、構築は $O(n(n+m)\log(n+1))$ 時間と $O(n+m)$ 一時領域を使い、構築後は $O(n)$ 領域です。

## 更新・参照

```text
std::uint64_t set_star_weight(int v, std::uint64_t weight);
std::uint64_t minimum_cut() const noexcept;
std::uint64_t star_weight(int v) const;
std::size_t size() const noexcept;
```

`set_star_weight` は辺 `(v,n)` の重みを変更し、更新後の全域最小カットを $O(\log^2(n+1))$ 時間で返します。元の辺の重みは固定です。残り3つは最小カットの重み、指定辺の重み、元の頂点数をそれぞれ $O(1)$ 時間で返します。

## 使用例

```cpp
const std::array edges{sirkit::DynamicStarCutEdge{0, 1, 5}};
sirkit::DynamicStarMinCut cut(2, edges);
cut.set_star_weight(0, 2);   // 2
cut.set_star_weight(1, 10);  // 7
```

[使用例](../Examples/dynamic_star_min_cut_example.cpp)・[Library Checker Verify](../Verify/current/submissions/dynamic_star_min_cut.cpp)
