# Dijkstra

[実装](../Graph/Dijkstra.cpp)・[使用例](../Examples/dijkstra_example.cpp)

## 対象と前提

非負重みのグラフで、1つの始点から全頂点への最短距離を求めます。
`WGraph`は`std::vector<std::vector<Edge>>`、辺は`Edge{int to, std::int64_t w}`です。重みの範囲は`0<=w<=1'000'000'000`、頂点番号はグラフの範囲内で指定します。
各呼び出しの末尾に正の`std::int64_t inf`を指定できます。省略時は`kInf = std::int64_t{1} << 60`です。
未到達の距離は`inf`、有限な最短距離が`inf`以上となる場合は`overflow_error`です。

V=頂点数、E=辺数、L=返す経路の頂点数です。以下は全てO(V+E log(E+1))時間・O(V+E)追加領域です。

### Dijkstra

```text
inline void Dijkstra(const WGraph &graph, std::vector<std::int64_t> &distances, int start_index, std::int64_t inf = (std::int64_t{1} << 60));
inline void Dijkstra(const WGraph &graph, std::vector<std::int64_t> &distances, int start_index, int target_index, std::vector<int> &path, std::int64_t inf = (std::int64_t{1} << 60));
inline void Dijkstra(int n, std::span<const std::tuple<int,int,std::int64_t>> edges, std::vector<std::int64_t> &distances, int start_index, std::int64_t inf = (std::int64_t{1} << 60));
inline void Dijkstra(int n, std::span<const std::tuple<int,int,std::int64_t>> edges, std::vector<std::int64_t> &distances, int start_index, int target_index, std::vector<int> &path, std::int64_t inf = (std::int64_t{1} << 60));
```

全頂点の距離を`distances`へ上書き 全頂点の距離と、始点から終点までの頂点列を上書き tupleの辺列から全頂点の距離を上書き tupleの辺列から全距離と終点までの頂点列を上書き

**制約:** 頂点・重み・infと有限距離の範囲は下記の前提を満たすこと。

**計算量:** O(V+E log(E+1))時間・O(V+E)追加領域。経路出力のO(L+1)も含みます。


経路復元のO(L+1)時間・領域も上の計算量に含みます。終点が到達不能なら`path`は空です。経路版も終点への到達時には打ち切りません。

辺列版の`edges`は`(u,v,w)`が`u→v`と重みを表す`std::tuple<int,int,std::int64_t>`の連続列です。`std::span<const std::tuple<int,int,std::int64_t>>`で受けるので、同じ型の`vector`も渡せます。入力が辺列として揃っている場合に使い、内部でO(V+E)の隣接表を構築します。呼出し中は入力を変更しません。

```cpp
std::vector<std::tuple<int,int,std::int64_t>> edges{{0,1,2},{1,2,3}};
std::vector<std::int64_t> distance;
std::vector<int> path;
Dijkstra(3, edges, distance, 0, 1, path);
// distance={0,2,5}, path={0,1}。終点より先の距離も確定する。
```

```cpp
const std::int64_t inf = 1'000'000'000'000;
Dijkstra(3, edges, distance, 0, inf); // 到達不能はこのinfになる。
```

最小距離の頂点の出辺を緩和し、改善した距離を待ち行列へ入れます。
非最短の経路が`inf`を超えていても、全ての有限な最短距離が範囲内なら正常に返ります。

## 計算量と検査

頂点数`V`、辺数`E`に対し、時間はO(V+E log(E+1))、追加領域はO(V+E)です。
距離のみの呼び出しでは経路用配列を作りません。入力のグラフは変更しません。

入力検査は既定で有効です。`WGraph`版は始点・終点と、処理中に参照した辺の行先・重みを検査し、未到達成分の全辺を検査する走査は行いません。
辺列版は構築時に全辺を参照し、両端点と重みを検査します。不正な頂点・重み・始点・終点は`out_of_range`、負の`n`や容量超過は`length_error`です。
`SKIP_RUNTIME_CHECKS`を定義する場合も、重み・頂点番号の前提は同じです。容量上限や有限距離と`inf`の衝突への対策は維持します。`inf<=0`は検査省略時も`invalid_argument`です。

## 構築・状態と補助型

### Edge

```text
Edge(int destination, std::int64_t weight);
```

行先toと重みwを持つ辺を作ります。引数なしでは両方0です。

**制約:** グラフへ追加して使うときは各最短路APIの頂点・重みの前提に従います。

**計算量:** O(1)。
