# グラフの近傍への一括代入

無向グラフの頂点に値を持たせ、ある頂点と隣接する頂点の値をまとめて上書きします。頂点ごとに現在値を取得できます。[本体](../Graph/Neighborhood_Assignment.cpp)

| 操作 | 意味 | 時間 |
| --- | --- | --- |
| `NeighborhoodAssignment<T>(initial, edges, threshold = 0)` | 初期値と辺の列から構築 | $O(n+m)$ |
| `get(v)` | 頂点 `v` の現在値を返す | $O(1+m/B)$ |
| `assign(v, x)` | 頂点 `v` に `x` を代入 | $O(1)$ |
| `assign_neighbors(v, x, include_self = false)` | `v` と隣接する頂点に `x` を代入。`include_self` が真なら `v` も含める | $O(1+B)$ |
| `size()` | 頂点数を返す | $O(1)$ |

頂点番号は $0$ 始まりで、辺は `std::pair<int, int>` です。自己辺があれば `v` 自身も隣接頂点に含みます。多重辺も扱えます。

しきい値 $B$ を省略すると $\lfloor\sqrt{2m}\rfloor+1$ です。次数が $B$ を超える頂点では、更新内容を記録し、取得時に反映します。領域は $O(n+m)$ で、表の時間は値のコピーを $O(1)$ として数えます。

## 使用例

```cpp
const std::array initial{1, 2, 3};
const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
sirkit::NeighborhoodAssignment<int> colors(initial, edges);
colors.assign_neighbors(1, 8);  // 0と2が8になり、1は2のまま。
```

[使用例](../Examples/neighborhood_assignment_example.cpp)・[Verify](../Verify/current/submissions/neighborhood_assignment.cpp)
