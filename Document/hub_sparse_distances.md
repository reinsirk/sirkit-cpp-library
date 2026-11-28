# ハブと少数の追加辺があるグラフの最短距離

任意の2頂点 `u,v` を費用 `gates[u]+gates[v]` で結びます。たとえば、各都市と共通のハブとの移動に `gates[i]` かかる場合に使えます。

- `HubSparseDistances(gates, shortcuts)` は追加の無向辺 `Shortcut{from,to,cost}` も含めて前処理します。頂点番号は0始まりです。費用は非負で、`gates[i]` は `UINT64_MAX/2` 以下です。
- `distance(u,v)` は最短距離を `uint64_t` で返します。`size()` は頂点数を返します。どちらも $O(1)$ 時間です。

費用0の辺を縮約し、ハブ経由以上の費用がかかる辺を取り除きます。残った辺の端点数を $k$、元の頂点数と追加辺数を $n,m$ とすると、前処理は時間 $O(1+(n+m)\alpha(n+1)+k^3)$、領域 $O(1+n+k^2)$ です。

[使用例](../Examples/hub_sparse_distances_example.cpp) ／ [Verify](../Verify/current/submissions/hub_sparse_distances_yuki3810.cpp)
