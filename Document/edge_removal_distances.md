# 辺を1本除外した後の最短距離

始点と終点を固定し、各辺を1本ずつ使えなくした場合の最短距離を求めます。[本体](../Graph/Edge_Removal_Distances.cpp)

```text
edge_removal_distances(n, edges, source, target, directed = true)
    -> std::vector<int>;
```

頂点番号は $0$ 始まりで、辺は `std::pair<int, int>` です。各辺の長さは $1$ です。無向グラフには `directed = false` を指定し、各辺を一度だけ渡します。自己辺と多重辺も扱えます。

結果の `i` 番目は入力の辺 `i` だけを除いたときの最短距離です。到達不能なら `-1` を返します。

最短路を始点からの距離で区切り、各段で最短路が通れる辺を数えます。通れる辺が1本しかない段では、その辺を除くと現在の最短路がなくなるので、幅優先探索をやり直します。該当する辺が $b$ 本なら $O((b+1)(n+m))$ 時間、$O(n+m)$ 領域です。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{0, 2}, std::pair{1, 2}};
const auto result = sirkit::edge_removal_distances(3, edges, 0, 2);
// {1, 2, 1}。辺 0→2 を除くと 0→1→2 の長さが2。
```

[使用例](../Examples/edge_removal_distances_example.cpp)・[Verify](../Verify/current/submissions/edge_removal_distances.cpp)
