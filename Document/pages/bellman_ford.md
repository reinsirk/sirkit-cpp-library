# 単一始点最短路（Bellman–Ford）

`Bellman-Ford.cpp`は`BellmanFord(graph, distances, start, inf=kInf)`を提供します。負辺を許す有向グラフの全頂点への最短距離を求め、始点から到達できる負閉路があれば`true`を返します。このファイルだけでは`SPFA`は呼べません。

## BellmanFord

```text
inline bool BellmanFord(const WGraph& graph, std::vector<std::int64_t>& distances, int start_index, std::int64_t inf = (std::int64_t{1} << 60));
```

全頂点への距離をdistancesへ上書きし、始点から到達できる負閉路があればtrueを返します。

**制約:** 始点と辺の行先が有効で、辺重み・inf・有限距離は以下の前提を満たすこと。

**計算量:** V頂点・E辺に最悪O(VE+V+E)時間・O(V)追加領域。

## 使用例

```cpp
WGraph graph(4);
graph[0].emplace_back(1, -2);
graph[1].emplace_back(2, 5);
std::vector<std::int64_t> distance;
const bool negative = BellmanFord(graph, distance, 0);
// negative=false、distance={0,-2,3,kInf}。
```

`WGraph`は`std::vector<std::vector<Edge>>`、辺は`Edge{int to, std::int64_t w}`です。計算に成功すると`distances`全体を上書きします。始点から到達できない頂点の距離は`inf`、負閉路を経由して到達できる頂点の距離は`-inf`です。始点から到達できない成分の負閉路は、戻り値に影響しません。

## 前提と計算量

頂点数をV、辺数をEとして、最悪O(VE+V+E)時間・O(V)追加領域です。入力グラフは変更せず、内部の作業領域は呼び出しごとに所有します。

辺重みは`[-1e9,1e9]`、Vは`INT_MAX`以下、始点と辺の行先は有効な頂点番号とします。`inf>0`で、負閉路の影響を受けない最終的な有限距離が`(-inf,inf)`に収まる必要があります。正値でない`inf`は`invalid_argument`、有限距離が範囲外なら`overflow_error`です。

既定では始点と計算中に参照する辺だけを検査し、例外時も既存の距離配列を保ちます。未到達成分を検査のためだけに走査しません。`SKIP_RUNTIME_CHECKS`では入力検査を除き、例外時の出力は未規定です。容量・算術範囲・`inf`の正値の保護は維持します。

全APIの引数と戻り値は、このページのソースから生成した一覧で確認できます。詳しい前提・動作と方式の比較は[負辺を含む最短路](../negative_shortest_paths.md)、完全な例は[使用例](../../Examples/bellman_ford_example.cpp)を参照してください。

## 構築・状態と補助型

### Edge

```text
Edge(int destination, std::int64_t weight);
```

行先toと重みwを持つ辺を作ります。引数なしでは両方0です。

**制約:** グラフへ追加して使うときは各最短路APIの頂点・重みの前提に従います。

**計算量:** O(1)。
