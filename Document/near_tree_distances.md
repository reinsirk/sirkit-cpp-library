# 木に近い無向グラフの最短距離

木に少数の辺を足したような無向グラフで、2頂点間の最短距離を繰り返し求めます。[本体](../Graph/Near_Tree_Distances.cpp)

- `NearTreeDistances(n, edges)` に、$0$ 始まりの頂点数と `std::pair<int, int>` の辺列を渡します。各辺の長さは $1$ です。非連結グラフ、自己辺、多重辺も扱えます。
- `distance(u, v)` は最短路の辺数です。非連結なら `-1` を返します。`size()` は頂点数を $O(1)$ で返します。

全域森から外れた辺のうち、自己辺と森の辺と端点が同じ辺を除き、少なくとも片方の端点から幅優先探索します。最短路が森の外の辺を通る場合、その端点のどれかを経由するからです。

成分 $i$ の頂点数を $n_i$、辺数を $m_i$、余分な辺数を $k_i=m_i-n_i+1$ とします。前処理は $O(\sum_i(k_i+1)(n_i+m_i))$ 時間、$O(m+\sum_i(k_i+1)n_i)$ 領域です。同じ成分の距離は $O(k_i+\log(n_i+1))$、非連結の判定は $O(1)$ です。

## 使用例

```cpp
using Edge = std::pair<int, int>;
const std::array edges{Edge{0, 1}, Edge{1, 2}, Edge{2, 3}, Edge{3, 4}, Edge{4, 0}};
const sirkit::NearTreeDistances distance(6, edges);
// distance.distance(1, 4) == 2, distance.distance(1, 5) == -1
```

[使用例](../Examples/near_tree_distances_example.cpp)・[Verify](../Verify/current/submissions/near_tree_distances.cpp)
