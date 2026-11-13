# 強連結成分分解

`scc_graph`は有向グラフを強連結成分に分けます。成分を1頂点にまとめると閉路のない有向グラフ（縮約DAG）になります。[本体](../Graph/SCC.cpp)・[使用例](../Examples/scc_example.cpp)を参照してください。

型名・コンストラクタ・`add_edge(int,int)`・`scc()`の呼び出しと戻り値は[ACL](https://atcoder.github.io/ac-library/production/document_ja/scc.html)に合わせています。このライブラリでは型をグローバルに置き、`atcoder::`は付けません。

## API

| API | 動作 |
| --- | --- |
| `scc_graph()` / `(int n)` / `(const Graph& graph)` | 空／n頂点で準備／Graphから構築・分解する。 |
| `add_edge(int u,int v)` | 有向辺を追加する。 |
| `scc()` | 分解し、トポロジカル順の`std::vector<std::vector<int>>`を返す。群内は頂点番号の昇順。 |
| `build()` | 成分IDだけが必要な場合に、頂点群を作らず分解して成分数を返す。 |
| `id(v)` / `ids()` / `component_count()` | 頂点vの成分番号／番号配列のconst参照／成分数。各O(1)。 |
| `condensation(unique=true)` | 縮約Graphを返す。自己辺を除き、既定では多重辺も除く。 |
| `size()` / `edge_count()` / `reserve_edges(m)` | 頂点数／辺数／辺容量を予約。 |
| `reset(n=0)` / `swap(other)` | 空のn頂点へ戻す／O(1)交換。 |

```cpp
scc_graph graph(3);
graph.add_edge(0,1);
graph.add_edge(1,0);
graph.add_edge(1,2);
auto groups = graph.scc(); // {{0,1},{2}}。
bool same = graph.id(0)==graph.id(1); // true。
Graph dag = graph.condensation();
```

頂点番号は0..N-1で、自己辺・多重辺を許します。`scc()`は呼ぶたびに分解し直します。成分IDや縮約グラフの照会は`scc()`または`build()`後に行い、辺を追加したら再分解します。成分番号は縮約DAGのトポロジカル順で、異なる成分間の辺u→vではid(u)<id(v)です。

`scc()`・`build()`はO(N+M)時間・追加領域です。各頂点の出次数が1なら、経路をたどって閉路と流入部分を分けます。それ以外は反復Tarjan法を使い、深い鎖でも再帰スタックを使いません。辺追加は償却O(1)、`condensation()`はO(N+M)時間・追加領域です。

コピーは独立、move元は空の未分解グラフです。`build()`中の確保失敗は旧結果を保ちますが、追加済みの辺は取り消しません。`scc()`の頂点群作成に失敗した場合、分解結果は更新されていることがあります。`ids()`の参照は再分解・代入・破棄で無効になります。既定では端点と結果の有効性を検査し、`SKIP_RUNTIME_CHECKS`で除きます。内部容量・添字上限は維持します。

[独立参照](../Verify/current/tests/new_scc_two_sat_reference_test.cpp)・[Library Checker](https://judge.yosupo.jp/problem/scc)
