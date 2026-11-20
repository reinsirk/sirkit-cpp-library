# グラフ文書から分離した詳細

2026-11-15に説明を整理する前の文書です。公開APIの現在の使い方は各Documentを参照してください。内部処理・出典・検証の説明は利用ガイドから分け、ここに原文のまま残します。

## Document/topological.md

# トポロジカルソート

有向グラフの頂点を、各辺`u→v`でuがvより先になるように並べます。処理の依存関係を満たす順序が必要なときに使います。閉路があるグラフでは、この順序を作れません。[使用例](../Examples/topological_example.cpp)と[辞書順最小の例](../Examples/topological_lexicographical_example.cpp)を参照してください。

`Graph/Topological.cpp`と`Graph/Topological_sort.cpp`は同じ公開API・共有ガードを持ち、どちらか一方を貼り付けて使えます。Graphは`std::vector<std::vector<int>>`で、graph[u]にuからの行先を入れます。

## API

| API | 戻り値・用途 |
| --- | --- |
| `TopologicalSort(graph)` | 入次数0の頂点をFIFOで処理した順序。閉路があれば空列。 |
| `TopologicalSortLexicographical(graph)` | 作れる順序のうち辞書順最小のもの。閉路があれば空列。 |
| `TopologicalSortDetails(graph, lexicographical=false)` | 順序・各頂点の位置・DAGか・順序が一意かをまとめて返す。trueなら辞書順。 |
| `IsDAG(graph)` | 有向閉路がなければtrue。空グラフもtrue。 |

Detailsの戻り値は`TopologicalResult`で、orderが頂点列、position[v]が列中の位置、acyclicが閉路なし、uniqueが順序の一意性です。空グラフは順序が空・acyclicとuniqueはtrueです。

閉路がある場合もDetailsは処理できた頂点をorderへ残し、未処理のpositionは-1、uniqueはfalseになります。閉路の下流の頂点も未処理になることがあるため、orderが閉路以外の全頂点を含むとは限りません。

```cpp
const Graph graph{{3}, {2}, {}, {}}; // 辺0→3、1→2。
const auto fifo = TopologicalSort(graph);                // 0,1,3,2。
const auto lex = TopologicalSortLexicographical(graph);  // 0,1,2,3。
const auto result = TopologicalSortDetails(graph, true);
// result.acyclic == true、result.unique == false。
// result.position[2] == 2。
```

## 計算量と前提

入次数0の頂点を取り出し、その頂点から出る辺の入次数を減らす方法です。FIFO版はO(V+E)時間、辞書順版はO(E+V log(V+1))時間、追加領域はどちらもO(V)です。Detailsは選んだ順序の方式と同じ計算量です。入力は変更せず、戻り値の配列は独立して所有します。

頂点数はINT_MAX以下、各入次数はUINT32_MAX以下、辺の終点は`[0,V)`が前提です。終点の検査は通常の入次数集計中に行い、`SKIP_RUNTIME_CHECKS`で省きます。容量の保護は検査省略時も維持します。

対応するGCC／Clang・CPUでは初期候補の登録にSIMDを使う場合があります。`SIRKIT_DISABLE_SIMD`で専用処理とCPU判定を除き、標準C++23の処理を使えます。候補管理や走査の最適化の詳細は[Audit](../Audit/)に置きます。

Verifyは通常版に[DP G](https://atcoder.jp/contests/dp/tasks/dp_g)、辞書順版に[ABC223 D](https://atcoder.jp/contests/abc223/tasks/abc223_d)を使います。閉路・一意性・部分順序などは独立ローカル検査で補います。[アルゴリズムの参考](https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/topological-sort)。


## Document/centroid_decomposition.md

# 重心分解

`CentroidDecomposition`は、無向木を「取り除くと各成分が半分以下になる頂点＝重心」で分割し、O(log(N+1))段の重心木を作ります。頂点への印付けと最寄りの印の照会などに使えます。[本体](../Graph/Centroid_Decomposition.cpp)・[使用例](../Examples/centroid_decomposition_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `CentroidDecomposition(graph,store_distances=true)` / `build(...)` | 分解を構築／再構築する。入力を変更・保持しない。 |
| `root()` / `parent(v)` / `depth(v)` | 重心木の根／親／深さ。根の親、空の根は-1。 |
| `component_size(v)` | vを重心として除去した時点の成分サイズ。 |
| `ancestors(v)` | 重心木の根からvまでのAncestor列へのconst参照。最後は{v,0}。 |
| `distance(u,v)` | 元の木の辺数距離をstd::int64_tで返す。 |
| `parents()` / `depths()` / `order()` | 結果配列へのconst参照。orderでは親が子より先。 |
| `centroid_tree()` | 重心木の無向隣接リストを所有値で返す。 |
| `size()` / `empty()` / `stores_distances()` | 頂点数／空か／距離を保存しているか。 |
| `clear()` / `swap(other)` | 空にする／結果を交換する。 |

```cpp
Graph graph{{1}, {0,2}, {1,3}, {2}};
CentroidDecomposition decomposition(graph);
auto distance = decomposition.distance(0,3); // 3。
for (auto a : decomposition.ancestors(3)) {
    // a.vertexは祖先の重心、a.distanceは元の木での3からの距離。
}
```

## 距離情報の使い方

Ancestorは{vertex,distance}です。距離は重心木ではなく**元の木での辺数**で、各祖先列の長さはO(log(N+1))です。最寄りの印を管理する場合、各重心について「印からの最小距離」を持ち、印付け・照会で祖先列だけをたどります。削除を行わない場合、各操作はO(log(N+1))です。具体的なコードは使用例にあります。

既定では距離情報も構築し、O(N log(N+1))時間・領域です。`CentroidDecomposition(graph,false)`または`build(graph,false)`なら距離を保存せず、領域O(N)です。この場合ancestors・distanceは使えません。distanceはO(log(N+1))、centroid_treeはO(N)、結果配列の参照・単点取得はO(1)です。

## 入力・所有

頂点0..N-1の単純無向木が前提です。空の木も許します。全探索は反復で、長い鎖でもNに比例する再帰スタックを使いません。入力は構築後に破棄・変更できます。

コピーは独立、move元は空、swapはO(1)です。コピー代入・buildに失敗した場合は旧結果を維持します。結果への参照は再構築・代入・clear・破棄まで有効です。

既定では初回の成分探索に、頂点番号・循環・到達頂点数・逆向きの辺の検査を組み込みます。検査専用の全辺走査や、後の成分での繰り返し検査はしません。`SKIP_RUNTIME_CHECKS`でこれらと頂点・距離保存の検査を除き、頂点数の表現上限は維持します。

数学的参考は[Nyaan](https://nyaannyaan.github.io/library/tree/centroid-decomposition.hpp.html)、機能上の参考は[Luzhiled](https://ei1333.github.io/library/graph/tree/centroid-decomposition.hpp.html)です。入力を保持しない反復構築として独立に記述しています。


## Document/max_flow.md

# 最大流

`mf_graph<Cap>`は、非負整数容量の有向グラフの最大流を求める。型名・辺の型・引数と戻り値・呼び出し形式は[ACLの最大流](https://atcoder.github.io/ac-library/production/document_ja/maxflow.html)に合わせている。独自の名前空間は不要で、`Graph/Max_Flow.cpp`を単独で貼り付けられる。ACL実装の転記ではない。

`Cap`には容量と追加流量が収まる整数型を渡す。ACLの`int`・`std::int64_t`に加え、bool以外の符号なし整数型・小さい整数型も使用できる。省略時は`std::int64_t`。

## API

`n`は頂点数、`m`は追加した元辺の数。頂点は`[0,n)`、元辺番号は追加順の`[0,m)`で、`m <= INT_MAX`。

| 呼び出し | 引数・戻り値 | 計算量 |
| --- | --- | --- |
| `mf_graph<Cap>()` / `mf_graph<Cap>(n)` | 空 / `n`頂点のグラフを作る | O(1) / O(n) |
| `add_edge(from,to,cap)` | 非負容量の元辺を追加し、`int`の辺番号を返す | 償却O(1) |
| `get_edge(int id)` | `edge{int from,to; Cap cap,flow;}`のコピー | O(1) |
| `edges()` | 元辺を追加順に並べた`vector<edge>` | O(m) |
| `change_edge(int id,new_cap,new_flow)` | 元辺の容量・流量を変更する | O(1) |
| `flow(s,t)` | 追加で流せる量を`Cap`で返す | O(n²m) |
| `flow(s,t,flow_limit)` | 上限まで追加で流し、その量を返す | O(n²m) |
| `min_cut(s)` | 残余グラフで`s`から到達可能かを`vector<bool>`で返す | O(n+m) |
| `size()` / `edge_count()` | 頂点数`int` / 元辺数`size_t` | O(1) |
| `reserve_edges(m)` | 元辺の位置表を予約する | O(m)以下の確保・移動 |
| `reset(n=0)` / `swap(other)` | 空の`n`頂点へ戻す / 状態を交換する | O(n+旧領域の破棄) / O(1) |

`get_edge`・`edges`・`min_cut`はACLと同じ非const呼び出しに加え、constオブジェクトからも同名で呼べる。`size`・`edge_count`・`reserve_edges`・`reset`・`swap`は追加機能。

## 使用例

```cpp
mf_graph<std::int64_t> graph(4);
const int id = graph.add_edge(0, 1, 5);
graph.add_edge(1, 3, 5);
const auto first = graph.flow(0, 3, 2); // 2だけ流す
const auto rest = graph.flow(0, 3);     // 残り3を流す
const auto used = graph.get_edge(id);  // used.cap == 5, used.flow == 5
const auto cut = graph.min_cut(0);     // cut[0] == true, cut[3] == false
```

[完全な使用例](../Examples/max_flow_example.cpp) · [Verify提出用コード](../Verify/current/submissions/max_flow_grid.cpp)

## 前提と所有

- 自己辺・多重辺・逆向きの元辺も追加できる。`change_edge`は`0 <= new_flow <= new_cap`が前提で、他の頂点の流量保存は利用側で保証する。
- `flow`の戻り値は累積ではなく今回の追加量。同じ始点・終点で継続でき、指定上限は非負とする。既定の上限は`Cap`の最大値で、総流量が型に収まらない場合は複数回に分け、累積値を十分に広い型で保持する。
- ACLでは`s != t`が前提。この実装は追加仕様として`s == t`を0とし、`flow_limit == 0`も0を返す。異なる端点で継続する場合も、利用側で現在の流量保存を保証する。
- `min_cut`が最小カットを表すのは、最大流を最後まで流して`t`へ到達できなくなった後。上限で途中停止した状態では最小とは限らない。
- 再帰しないDinic法を用い、グラフと作業領域はO(n+m)。コピー構築・代入はO(n+m)で残余を独立所有する。移動構築はO(1)で、移動元は空になる。代入は自身への参照を返し、旧領域の破棄費用が加わる。コピー代入・resetの失敗時は元を保持する。
- 入力検査は操作時に行い、`SKIP_RUNTIME_CHECKS`で除く。内部の辺数・添字上限と安全な容量差の計算は維持する。辺追加の確保失敗で片方向辺を残さず、`flow`の作業領域は残余更新前に確保する。

[独立参照試験](../Verify/current/tests/new_max_flow_reference_test.cpp) · [ACL互換・境界試験](../Verify/current/tests/max_flow_acl_api_test.cpp) · [AtCoder practice2 D](https://atcoder.jp/contests/practice2/tasks/practice2_d)


## Document/min_cost_flow.md

# 最小費用流

`MinCostFlow<Capacity,Cost>`は、有向グラフで指定量までの流れを最小費用で追加します。頂点は0..N-1、容量は非負整数で、費用は負でも構いません。既定は両方std::int64_tです。[本体](../Graph/Min_Cost_Flow.cpp)・[使用例](../Examples/min_cost_flow_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `MinCostFlow(n=0)` / `add_edge(u,v,capacity,cost)` | n頂点で構築／辺を追加してsize_tの辺IDを返す。自己辺・多重辺可。 |
| `flow(s,t,limit)` | limit以下の追加流量と追加費用のpairを返す。limit省略時はCapacityの最大値。 |
| `slope(s,t,limit)` | {0,0}からの追加流量・費用の折れ線を返す。限界費用が同じ区間はまとめる。 |
| `get_edge(id)` / `edges()` | Edge{from,to,capacity,flow,cost}のコピー／元辺順の一覧。 |
| `change_edge(id,capacity,flow)` / `change_cost(id,cost)` | 指定辺の状態／費用を変える。 |
| `reset_flow()` / `min_cut(s)` | 全元辺の流量を0へ／現在の残余で到達可能な頂点のbool配列。 |
| `size()` / `edge_count()` / `reserve_edges(m)` | 頂点数／元辺数／元辺位置表の予約。 |
| `reset(n=0)` / `swap(other)` | 空のn頂点へ戻す／交換。 |

```cpp
MinCostFlow<> graph(2);
graph.add_edge(0,1,2,-3);
graph.add_edge(0,1,4,5);
auto [amount,cost] = graph.flow(0,1,1); // 追加量1、費用-3。
auto [more,extra] = graph.flow(0,1);    // 追加量5、費用17。
```

戻り値は**その呼び出しで追加した量と費用**で、累積値ではありません。同じ始点・終点で続けて呼べます。s==tまたはlimit==0では流しません。始点・終点の変更やchange_edge後も続ける場合は、0<=flow<=capacityと既存流の流量保存を利用側で保証してください。

## 負費用・計算量

全頂点へ到達する仮想始点からBellman–Ford法でポテンシャルを求め、換算した非負費用でDijkstra法を繰り返します。正の残余容量を持つ辺の負閉路は、sから到達できなくてもdomain_errorで拒否します。有限容量の負閉路でもcycle cancelingは行いません。検出前に費用範囲を超えればoverflow_errorになります。

初期化は負費用辺がなければO(N+M)、一般にはO(NM)。増流A回でO(NM+A(N+M)log(N+M+1))時間、領域O(N+M)です。再帰せず、作業領域は再利用します。辺の変更はポテンシャルを無効にし、次回に再計算します。辺追加は償却O(1)、単点取得・変更はO(1)、一覧・reset_flowはO(M)、min_cutはO(N+M)です。

## 数値・所有と例外

Capacityはbool以外の整数、Costは符号付き整数で、いずれも64ビット以下です。辺費用は[-max(Cost),max(Cost)]。Costの最小値は逆辺の符号反転ができないため、検査省略時もoverflow_errorです。ポテンシャル・換算費用・距離候補・限界費用・追加費用の**途中計算もCostに収まること**が必要です。最終解だけが収まる条件では足りません。

GCC/Clangの128ビット整数拡張が利用可能なら、費用の積和等の範囲判定に使います。これは標準C++の型ではありません。`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準C++の代替演算へ切り替えられます。費用範囲・負閉路の判定は`SKIP_RUNTIME_CHECKS`でも維持し、端点・負容量等の入力検査だけを省きます。

コピーは残余状態を独立所有し、move元は空、swapはO(1)です。flow・slopeの例外時にも、完了した増流が残る場合があります。途中の増流を片方向だけ更新せず、ポテンシャル更新の失敗時はキャッシュを無効化します。slopeの結果配列の確保失敗も、完了済み増流は取り消しません。

[独立参照](../Verify/current/tests/new_min_cost_flow_reference_test.cpp)・[AtCoder practice2 E](https://atcoder.jp/contests/practice2/tasks/practice2_e)


## Document/scc.md

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


## Document/heavy_light_decomposition.md

# 重軽分解

`with_values<Range>(values)`は、木の分解と選んだBIT／Segtreeを所有する`HeavyLightValues`を返します。初期値は**values[v]が頂点vの値**になる順で渡し、操作も頂点番号を使います。並べ替え・添字変換は内部で行います。[本体](../Graph/Heavy_Light_Decomposition.cpp)・[完全な使用例](../Examples/heavy_light_decomposition_example.cpp)。

このファイルはBIT／Segtree本体を含みません。次の例には[通常版Fenwick Tree](../DataStructure/Fenwick_Tree.cpp)も貼り付けます。分解結果だけを使う場合はHLD単独で利用できます。

```cpp
Graph tree{{1}, {0,2}, {1}}; // 0―1―2、根は既定で0。
std::vector<std::int64_t> value{10,20,30};
auto hld = HeavyLightDecomposition(tree)
    .with_values<fenwick_tree<std::int64_t>>(value);
hld.add(1, 5);                       // 頂点1を20→25にする。
auto sum = hld.path_prod(0, 2);       // 0→1→2の和、65。
auto subtree = hld.subtree_prod(1);  // 頂点1とその子孫の和、55。
auto x = hld.get(1);                 // 25。value[1]自体は20のまま。
```

## 集計器と演算の選び方

| Rangeに指定する型 | 向いている操作 |
| --- | --- |
| `fenwick_tree<T>` | 頂点への加算と経路・部分木の和。`get/set/add`が使える。 |
| `segtree<S,op,e>` | 頂点の代入と経路・部分木の最小値・最大値・積など。`get/set`が使える。 |
| `lazy_segtree<S,op,e,F,mapping,composition,id>` | 点・経路・部分木の一括更新も必要な場合。`get/set/apply`が使える。 |

Tは値、Sは各頂点に置く値と集計結果の型で、初期値はそれぞれvector<T>／vector<S>です。op(a,b)は2つの結果を結合し、e()は空区間の結果（左右の単位元）を返します。opには結合則が必要です。Fは更新内容の型、mapping(f,x)は更新後の集計値、composition(newer,older)は古い更新の後に新しい更新を行う合成、id()は何もしない更新を返します。[型引数の詳細](lazy_segment_tree.md)と完全例に範囲加算の定義があります。使う集計器も貼り付けます。実ACLも使用でき、Mintは利用側で用意します。

## 操作と経路の向き

以下、Nは頂点数、Mは辺数、Vは`HeavyLightValues<Range,Ordered>`、value_typeはBITならT、Segtree／LazyならSです。頂点・添字はint、祖先・jumpのkはstd::int64_t。通常の集計器で、値の演算・コピーをO(1)とした計算量です。

| VのAPI | 戻り値・動作 | 計算量 |
| --- | --- | --- |
| `value_type` | 保持する値・集計結果の型。 | — |
| `V()` | 値と分解が空の状態を作る。集計器は未構築。 | O(1) |
| `V(g,values,root=0)` | Graphと頂点順のvectorから構築する。 | 分解＋集計器構築 |
| `V(const HeavyLightDecomposition& layout,values)` / `V(HeavyLightDecomposition&& layout,values)` | 既存の分解をコピー／移動し、頂点順の値を取り込む。 | 分解のコピー／移動＋並べ替え＋集計器構築 |
| `V(const V&)` / `V(V&&)` / `operator=(V other)` | 独立コピー／移動構築／コピー・移動代入。代入は自身へのV&を返す。 | コピーは保持領域に比例、移動構築O(1) |
| `build(g,values,root=0)` | 分解と値を一緒に再構築。戻り値void。 | 分解＋集計器構築 |
| `clear()` / `swap(V& other)` | 空にする／同じ型と交換。戻り値void。 | 保持領域の解放／O(1) |
| `get(v)` / `set(v,x)` | value_typeで頂点vの値を返す／xを代入しvoid。 | getはSegtreeでO(1)、他はO(log N)。setはO(log N) |
| `add(v,x)` | 頂点vへ加算しvoid。addを持つBIT用。 | O(log N) |
| `apply(v,f)` | 頂点vへ作用しvoid。点applyを持つ集計器用。 | O(log N) |
| `path_prod(u,v,edges=false)` / `subtree_prod(v,edges=false)` | 両端を含む経路／vを含む部分木の結果をvalue_typeで返す。prodは和・最小値など、Rangeで決めた集計。 | O(log²N)／O(log N) |
| `path_apply(u,v,f,edges=false)` / `subtree_apply(v,f,edges=false)` | 経路／部分木へ作用しvoid。区間applyを持つ集計器用。 | O(log²N)／O(log N) |
| `values()` | 全値を頂点番号順のvector<value_type>で返す。 | Range.valuesの費用＋O(N)、なければN回get |

get・集計・valuesを含む値の操作は非constです。以下の「分解結果を取り出すAPI」はVからも同名で呼べます。Vにはwith_valuesや外部集計器を渡すオーバーロードはありません。

**既定のOrdered=falseは、順序を入れ替えても結果が同じ演算用**です。和・最小値・最大値はこのまま使えます。文字列連結・関数合成などはtrueにし、u→vの順に集計します。trueでは集計器2本分の領域・更新を使います。値の順を反転し、個々の辺の値を逆元にする機能ではありません。concatを文字列連結、empty_wordを空文字列を返す関数とすると、上の木で次のように使えます。

```cpp
using Words = segtree<std::string, concat, empty_word>;
std::vector<std::string> token{"a","b","c"};
auto words = HeavyLightDecomposition(tree).with_values<Words,true>(token);
auto forward = words.path_prod(0,2); // "abc"。
auto backward = words.path_prod(2,0); // "cba"。
```

**辺を扱うときは、親辺の値を子頂点の位置へ渡し、edges=trueにします。** 初期値はN個必要で、各根には0やe()を置きます。根0の0―1―2では、辺(0,1)をvalues[1]、辺(1,2)をvalues[2]へ置きます。`path_prod(0,2,true)`はこの2辺を集計し、`subtree_prod(1,true)`は部分木の内部の辺(1,2)だけを集計します。`set(2,x)`は親辺(1,2)の更新です。経路ではLCA、部分木では指定頂点の位置を除きます。`path_prod(v,v)`は頂点vの値、`path_prod(v,v,true)`は空経路の単位元です。葉の辺部分木も空で、空区間の更新は何もしません。非可換な部分木積はDFS添字順です。

## 分解の構築・所有

Hは`HeavyLightDecomposition`です。Graphは`std::vector<std::vector<int>>`の別名で、無向森の隣接リストを渡します。

| HのAPI | 戻り値・動作 | 計算量 |
| --- | --- | --- |
| `H()` / `H(g,root=0)` | 空／指定根からの分解を構築する。 | O(1)／O(N+M) |
| `H(const H&)` / `H(H&&)` / `operator=(H other)` | 独立コピー／移動構築／コピー・移動代入。代入は自身へのH&を返す。 | コピーO(N)、移動構築O(1) |
| `build(g,root=0)` / `clear()` / `swap(H& other)` | 再構築／空にする／交換。戻り値void。 | O(N+M)／旧領域の解放／O(1) |
| `with_values<Range,Ordered=false>(values) const &` | 長さNの頂点順vectorを受け、分解をコピーしてVを返す。 | コピー＋並べ替え＋集計器構築 |
| `with_values<Range,Ordered=false>(values) &&` | 同じ入力から、分解を移動してVを返す。 | 並べ替え＋集計器構築 |

コピー／moveの代入とbuildは、表の費用に旧状態の破棄費用も加わります。with_valuesは元の値配列の参照を保持しません。

## 分解結果を取り出すAPI（H・V共通）

この表の操作はconstから呼べます。区間のleft/rightは元の頂点番号ではなく、並べ替え後のDFS添字です。`H::Interval`は公開型`{int left; int right; bool reversed;}`で、Vに同名の型別名はありません。

| API | 戻り値・動作 | 計算量 |
| --- | --- | --- |
| `size()` / `empty()` | intの頂点数／boolの空判定。 | O(1) |
| `root()` / `component_count()` | intの指定根（空なら-1）／木の数。 | O(1) |
| `parent(v)` / `depth(v)` | intの親（各根で-1）／各根からの深さ（根で0）。 | O(1) |
| `component(v)` / `root_of(v)` | intの木の番号（0始まり）／所属する木の根。 | O(1) |
| `index(v)` / `vertex_at(i)` | intの頂点→DFS添字／添字→頂点。互いに逆変換。 | O(1) |
| `order()` | 添字iにvertex_at(i)を置くvector<int>のconst参照。 | O(1) |
| `reorder(values)` / `restore(ordered)` | 長さNのvector<T>をDFS添字順／頂点順へコピーして返す。Tはコピー可能なら既定構築不要。 | O(N)時間・追加領域 |
| `same_component(u,v)` / `is_ancestor(u,v)` | boolの同じ木か／uがvの祖先か。自身は祖先、別の木なら祖先ではない。 | O(1) |
| `subtree_range(v,edges=false)` | pair<int,int>の半開区間[l,r)。edges=trueはvの位置を除く。 | O(1) |
| `subtree_size(v)` | vを含む部分木の頂点数をintで返す。 | O(1) |
| `lca(u,v)` / `distance(u,v)` | intのLCA／経路の辺数。別の木なら-1。 | O(log N) |
| `kth_ancestor(v,k)` | k個上の頂点をintで返す。k=0はv、負数・根を超える場合は-1。 | O(log N) |
| `jump(u,v,k)` | u→vの0始まりk番目の頂点をintで返す。負数・範囲外・別の木は-1。 | O(log N) |
| `for_each_path(u,v,fn,edges=false)` | 通行順にfn(left,right,reversed)を呼び、接続ならtrue、別の木ならfalse。別の木では呼ばず、fnの戻り値は無視する。 | O(log N)回の呼出し、HLD側のヒープ確保なし |
| `path(u,v,edges=false)` | 同じ通行順のvector<H::Interval>を返す。別の木は空。 | O(log N)時間・領域 |

for_each_pathはfnの実行費用も加わります。reversed=trueはright-1からleftへ読みます。辺モードでu=vならfor_each_pathは呼出しなしでtrue、pathは空です。区間から頂点を取り出す場合はvertex_atを使います。

```cpp
HeavyLightDecomposition layout(tree); // BIT／Segtreeなしで使える。
auto [l,r] = layout.subtree_range(1); // vertex_at(l)..vertex_at(r-1)が部分木。
auto parts = layout.path(2,0);
std::vector<int> route;
for (auto [left,right,reversed] : parts)
    for (int i=0; i<right-left; ++i)
        route.push_back(layout.vertex_at(reversed ? right-1-i : left+i));
// 上の0―1―2の木ではrouteは{2,1,0}。
```

## 外部集計器を使うAPI（Hのみ）

Hはconstでも呼べます。bit／seg／lazyにはDFS添字順・長さNの集計器を渡します。参照は保持しません。Qを区間照会1回の費用、Aを区間更新1回の費用とします。

| API | 戻り値・前提 | 計算量 |
| --- | --- | --- |
| `path_prod(u,v,bit,edges=false)` | sumを持つBITの和。型はsumの戻り値、単位元はT{}。 | O(log N)回の照会 |
| `path_prod(u,v,seg,edges=false)` | ACL形式segtreeのop/eを使った可換積。segはconst可、戻り値S。 | O(log N)回の照会 |
| `path_prod(u,v,lazy,edges=false)` | ACL形式lazy_segtreeのop/eを使った可換積。lazyは非const、戻り値S。 | O(log N)回の照会 |
| `path_prod(u,v,seg,reverse,edges=false)` | segの非可換積。reverseは集計値を正しい逆順集計値へ変換する。戻り値S。 | O(log N)回の照会 |
| `path_prod(u,v,lazy,reverse,edges=false)` | lazyの非可換積。reverseの条件は同じ。lazyは非const、戻り値S。 | O(log N)回の照会 |
| `path_prod(u,v,identity,query,op,edges=false)` | 単位元・照会・結合演算を指定。queryの戻り値型で結果を返す。 | O(log N)回の照会・結合 |
| `path_prod(u,v,identity,query,op,reverse,edges=false)` | 上と同じで、集計値の逆順変換を追加。 | O(log N)回の照会・結合 |
| `subtree_prod(v,query,edges=false)` | 部分木のDFS添字昇順の積。戻り値型はqueryから決まる。 | Q |
| `path_apply(u,v,range,f,edges=false)` / `subtree_apply(v,range,f,edges=false)` | range.apply(l,r,f)で更新。戻り値void。fは値で受け取る。 | O(log N)回の更新／A |

queryはprod/sumを持つ集計器か、(l,r)／(l,r,reversed)から値を返す関数です。2引数は可換専用、3引数は向きに合った結果を返します。reverseと3引数queryは併用しません（std::identityは除く）。空区間は単位元を返してください。reverseは要素順を逆にした集計値への変換です。単一の結果だけで逆順を復元できない場合は3引数queryを使います。opは結合的でidentityを左右の単位元とし、区間照会・値のコピー・結合の費用を表の回数に加えます。

## 前提・所有権・計算量

入力は頂点0..N-1の対称な隣接リストで表す無向森です。指定根の木から構築し、残りは最小未訪問頂点を根にします。空の入力にはroot=0。経路集計・更新の両端は同じ木が前提で、検査時は別の木を例外にします。頂点範囲・自己辺・閉路・多重辺は探索中に検査し、対称性確認だけの全辺走査はしません。`SKIP_RUNTIME_CHECKS`で入力検査と検査専用状態を除きます。BITの整数和は型幅で周回します。

コピーは独立、move元は空です。元のlayout・値配列とも更新を共有しません。木の形や根を変えるときはbuildへ値も渡し、辺の値は新しい親子対応で配置し直します。build・コピー代入の失敗時は元を保持しますが、移動を指定した分解からの構築失敗では移動元が空になる場合があります。更新中の例外では巻き戻さず、Ordered=trueはclear/buildで再構築してください。pathが返す配列は独立所有、orderは参照です。clear・再構築・代入・破棄後も並びが必要ならコピーを保持してください。get・集計・valuesも非constのHLDから呼びます（Lazyは照会時も内部を変更します）。

並べ替えはO(N)、その後に集計器を構築します。vector構築のない実ACL FenwickはN回のaddでO(N log N)。所有状態を固定1回確保し、破棄時には所有領域を解放します。表の計算量は演算・コピーをO(1)と仮定し、文字列などは長さに応じた費用が加わります。

[API設計の参考（Nyaan・CC0）](https://github.com/NyaanNyaan/library/blob/2e713880f0d5041d794e0f09c7f31c1826ba0ccf/tree/heavy-light-decomposition.hpp)・[理論参考](https://info.atcoder.jp/entry/algorithm_lectures/heavy_light_decomposition)・[分解の参照試験](../Verify/current/tests/new_hld_reference_test.cpp)・[所有版の参照試験](../Verify/current/tests/hld_owned_test.cpp)


## Document/offline_dynamic_connectivity.md

# オフライン動的連結性

`OfflineDynamicConnectivity`は、無向グラフの辺追加・削除と照会を記録し、最後にsolveでまとめて解きます。連結判定・成分サイズ・成分数・成分の頂点重み和に対応します。[本体](../Graph/Offline_Dynamic_Connectivity.cpp)・[使用例](../Examples/offline_dynamic_connectivity_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `OfflineDynamicConnectivity(n)` / `(span<const int64_t> weights)` | n頂点・初期値0／指定した初期値で構築。 |
| `add_edge(u,v)` / `remove_edge(id)` | 辺を追加してsize_tのIDを返す／その辺を削除する。 |
| `add_vertex(v,delta)` | 以降の照会に対し頂点値へdeltaを加算する。 |
| `query_same(u,v)` / `query_size(v)` | その時点で連結か／成分サイズの照会を記録し、回答添字を返す。 |
| `query_component_count()` / `query_component_sum(v)` | 成分数／vの成分の頂点和の照会を記録し、回答添字を返す。 |
| `solve() const` | 照会順のvector<std::int64_t>を返す。連結判定は0/1。 |
| `size()` / `empty()` | 頂点数／0頂点か。 |
| `edge_count()` / `addition_count()` / `query_count()` | 削除済み込みの追加辺数／頂点加算数／照会数。 |
| `reserve_edges(n)` / `reserve_additions(n)` / `reserve_queries(n)` | 対応する記録容量を予約。 |
| `clear_history()` | 頂点数と構築時の初期値を残し、全ての辺・加算・照会を消す。 |
| `reset(n)` / `reset(weights)` / `clear()` | 再初期化／初期値を指定して再初期化／0頂点へ。 |

```cpp
OfflineDynamicConnectivity graph(4);
auto edge = graph.add_edge(0,1);
auto first = graph.query_same(0,1);
graph.remove_edge(edge);
auto second = graph.query_same(0,1);
auto answers = graph.solve(); // answers[first]==1、answers[second]==0。
```

## 履歴と計算量

solveは記録を消しません。何度でも実行でき、後から操作を追加して再solveできます。照会なしなら空vector、0頂点でも成分数の照会は可能で回答0です。clear_historyは加算後の値を新たな初期値にする操作ではありません。

辺が有効な期間を区間分割し、内部で巻き戻せるUnion-Findを使います。外部から任意時点へ戻すAPIはありません。頂点数N、追加辺数E、頂点加算数A、照会数Qとして、記録は償却O(1)、solveはO(N+(E+A+Q)log(Q+1)log(N+1))時間・O(N+(E+A)log(Q+1)+Q)作業領域です。構築はO(N)、件数の取得はO(1)です。

## 入力と所有

頂点は0..N-1で、自己辺・多重辺を許します。同じ両端の辺も別IDで、一方の削除は他方に影響しません。辺IDは有効な辺を削除するとき一度だけ使えます。無関係な別のオブジェクトのIDは渡さないでください。コピー時までのIDはコピー先でも同じ記録を指しますが、その後の操作は独立します。

頂点値は負でもよく、内部の加算順も含め全てstd::int64_tに収まることが前提です。初期化・履歴消去後の新しい記録には、古い辺ID・回答添字を流用しません。

コピーは全記録を独立所有し、コピーの費用は保持量に比例します。move元は0頂点・記録なし、move・swapはO(1)。確保失敗時は元の内容を保ち、constなsolveも記録を変更しません。既定では頂点番号と有効な辺IDを検査し、`SKIP_RUNTIME_CHECKS`で除きます。内部容量・添字保護は維持します。

[Library Checker Dynamic Graph Vertex Add Component Sum](https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum)と、連結判定・サイズ・再solve等の独立参照で検証します。


## Document/low_link.md

# Low-linkと二重連結成分

`UndirectedLowLink`は、無向多重グラフの橋・関節点・二重連結成分を求めます。橋は取り除くと連結成分が増える辺、関節点は取り除くと連結成分が増える頂点です。頂点番号は0始まりの`[0,n)`、辺は追加時のIDで区別します。

## API

| API | 動作・計算量 |
| --- | --- |
| `UndirectedLowLink(n=0)` / `reset(n=0)` | 空のn頂点グラフを作る／戻す。構築O(1)、resetは旧データの解放を含む。 |
| `reserve_edges(m)` / `add_edge(u,v)` | m辺分を予約／無向辺を追加し`size_t`の辺IDを返す。追加は償却O(1)。 |
| `size()` / `edge_count()` / `edge(id)` | 頂点数／辺数／端点の`pair<int,int>`を返す。O(1)。 |
| `build(collect_blocks=false)` | 分解して元のグラフの連結成分数を返す。O(n+m)。trueなら点二重連結ブロックも求める。 |
| `component_count()` | 最後のbuildの連結成分数。O(1)。 |
| `bridges()` / `is_bridge(id)` | 橋の辺ID群へのconst参照／橋かを返す。O(1)。 |
| `articulation_points()` / `is_articulation(v)` | 関節点番号群へのconst参照／関節点かを返す。O(1)。 |
| `two_edge_component_count()` | 2辺連結成分数を返す。O(1)。 |
| `two_edge_component(v)` / `two_edge_components()` | 橋を除いた成分番号／成分ごとの頂点群を返す。O(1)／O(n+m)。 |
| `bridge_forest()` | 2辺連結成分を頂点、橋を辺とした`Graph`を返す。O(n+m)。頂点番号は上記の成分番号。 |
| `biconnected_edge_components()` | 点二重連結ブロックごとの辺ID群へのconst参照。build(true)後、O(1)。 |
| `biconnected_vertex_components(include_isolated=true)` | 各ブロックの頂点群を返す。trueなら孤立頂点も単独群で加える。build(true)後、O(n+m)。 |
| `swap(other)` | グラフと分解結果をO(1)で交換する。 |

```cpp
UndirectedLowLink graph(4);
graph.add_edge(0,1); graph.add_edge(1,2); graph.add_edge(2,0);
auto edge = graph.add_edge(2,3);
graph.build(true);
bool bridge = graph.is_bridge(edge);              // true
int count = graph.two_edge_component_count();      // 2
auto groups = graph.two_edge_components();        // {0,1,2}と{3}
auto blocks = graph.biconnected_vertex_components();
auto forest = graph.bridge_forest();
```

[完全な使用例](../Examples/low_link_example.cpp)を参照してください。

## 前提と性質

結果の照会にはbuildが必要です。辺の追加後は再buildしてください。橋のIDと関節点番号は昇順ですが、成分番号・成分群やブロックの順序は規定しません。群を返す値は独立したコピーで、const参照は再build・reset・代入で無効になります。

平行辺は別IDとして処理します。自己辺はそれぞれ独立した1辺ブロックとし、同じ頂点だけの頂点ブロックが複数できる場合があります。孤立頂点は辺ブロックには含めません。この自己辺の扱いは、自己辺を許さないLibrary Checkerの点二重連結成分問題と区別してください。

DFSは明示スタックを使うため、深い鎖でも再帰しません。発見順と祖先へ戻れる最小の発見順から橋・関節点を求め、橋に当たらない木辺の親参照を圧縮して2辺連結成分をまとめます。build(true)では辺スタックで点二重連結ブロックも確定します。構築はO(n+m)時間・領域です。

コピーは独立所有、ムーブ元は空の未分解グラフです。build中に例外が出た場合は呼び出し前の状態を保ちます。`SKIP_RUNTIME_CHECKS`で頂点・辺番号・結果の有効性の検査を除きます。内部の容量保護や、橋・関節点を決める数学的な比較は維持します。

[独立参照](../Verify/current/tests/new_low_link_reference_test.cpp) · [確保失敗の検証](../Verify/current/tests/low_link_components_allocation_test.cpp) · [2辺連結成分](https://judge.yosupo.jp/problem/two_edge_connected_components) · [点二重連結成分](https://judge.yosupo.jp/problem/biconnected_components)


## Document/negative_shortest_paths.md

# 負辺を含む単一始点最短路

[標準版](../Graph/Bellman-Ford.cpp)・[SPFA版](../Graph/Bellman-Ford_SFPA.cpp)・[使用例](../Examples/bellman_ford_example.cpp)

負辺を含む有向グラフで、1つの始点から全頂点への最短距離を求めます。
`WGraph`は`std::vector<std::vector<Edge>>`、辺は`Edge{int to, std::int64_t w}`です。
両ファイルは単独でも、同時に貼り付けても利用できます。

## API

| 呼び出し | 結果 |
| --- | --- |
| `BellmanFord(graph, distances, start, inf=kInf)` | 全距離を上書きし、到達可能な負閉路があれば`true` |
| `SPFA(graph, distances, start, inf=kInf)` | 同じ結果を、更新頂点を再処理する方式で計算 |

`inf`は正の`std::int64_t`で、省略時は`kInf = std::int64_t{1} << 60`です。
未到達は`inf`、負閉路を経由して到達できる頂点は`-inf`になります。
未到達成分の負閉路では`true`になりません。負閉路から戻れない始点や、別の枝の距離は有限のままです。

```cpp
WGraph graph(4);
graph[0].emplace_back(1, -2);
graph[1].emplace_back(2, 5);
std::vector<std::int64_t> distance;
const std::int64_t inf = 1'000'000'000'000;
const bool negative = BellmanFord(graph, distance, 0, inf);
// negative=false, distance={0,-2,3,inf}
// SPFA(graph, distance, 0, inf)も同じ結果。
```

## 前提・例外

辺重みは`[-1e9, 1e9]`、頂点数は`INT_MAX`以下、始点と辺の行先は有効な頂点番号とします。
負閉路の影響を受けない有限の最短距離は`(-inf, inf)`に収まる必要があります。
これを外れる場合は`overflow_error`、`inf<=0`は`invalid_argument`です。
途中の経路の値が範囲を外れても、最終的な有限距離だけが判定対象です。
入力のグラフは変更しません。呼び出しごとに内部状態を所有します。

既定では始点と、計算中に参照する辺の終点・重みを検査し、不正なら`out_of_range`です。
未到達成分を検査のためだけに走査しません。容量超過は`length_error`です。
検査有効時は例外になっても既存の距離配列を保ちます。
`SKIP_RUNTIME_CHECKS`で入力検査を省く場合は、例外時の出力は未規定です。
容量・算術範囲・`inf`の正値の保護は検査省略時も残ります。

## 計算量と使い分け

V=頂点数、E=辺数とし、どちらも最悪O(VE+V+E)時間、O(V)追加領域です。
`BellmanFord`は辺の緩和を反復し、`SPFA`は更新された頂点を待ち行列で再処理します。
SPFAも最悪時間が改善するとは限りません。長い反復では到達部分の強連結成分ごとの処理へ移ります。

Verify先は[ABC137 E](https://atcoder.jp/contests/abc137/tasks/abc137_e)です。
未到達・全距離・負閉路の影響・カスタム番兵は独立参照でも照合します。


## Document/general_matching.md

# 一般グラフの最大マッチング

`GeneralMatching`は、無向グラフから端点を共有しない辺を最大何本選べるか求めます。奇数閉路を含むグラフにも使えます。重みは扱いません。[本体](../Graph/General_Matching.cpp)・[使用例](../Examples/general_matching_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `GeneralMatching(n=0)` / `add_edge(u,v)` | n頂点で構築／無向辺を追加。自己辺は無視し、多重辺は許す。 |
| `solve()` | 現在の対応を最大化し、総本数を返す。辺追加後も継続できる。 |
| `mate(v)` / `partners()` | 相手の番号（未対応は-1）／全対応のconst span。 |
| `pairs()` | u<vの対応辺をuの昇順でvectorにして返す。 |
| `matching_size()` / `is_maximum()` / `is_perfect()` | 現在の本数／最大性が確認済みか／全頂点が対応済みか。 |
| `size()` / `empty()` / `edge_count()` / `edges()` | 頂点数／0頂点か／自己辺以外の登録件数／追加順の辺表span。 |
| `reserve_edges(cap)` | 辺容量を予約する。 |
| `clear_matching()` / `clear_edges()` | 対応だけ消す／頂点数を残して辺と対応を消す。 |
| `reset(n=0)` / `clear()` / `swap(other)` | 再初期化／0頂点へ／交換。 |

```cpp
GeneralMatching graph(3);
graph.add_edge(0,1);
graph.add_edge(1,2);
graph.add_edge(2,0);
auto count = graph.solve(); // 1。
auto pairs = graph.pairs(); // 3辺のうちいずれか1辺。
```

## 対応と計算量

頂点番号は0..N-1で、左右への分割や端点の大小関係は不要です。多重辺は記録上は残しますが、対応では同じ頂点対を区別しません。辺を追加すると最大性は未確認になりますが、現在の対応は有効です。再solveで本数は減りませんが、相手が変わる場合があります。空グラフは最大かつ完全対応と扱います。

Edmonds法で奇数閉路を縮約しながら対応を増やします。探索・縮約は反復で、再帰しません。記録辺数MとしてsolveはO(N³+NM)時間・追加O(N+M)領域です。単純グラフではO(N³)。最大性が既知、または既に完全対応ならO(1)です。

件数・相手・span取得はO(1)、辺追加は償却O(1)、再確保はO(M)、pairsはO(N)時間・O(対応本数)領域。clear_matchingはO(N)、clear_edgesはO(N+M)。resetは旧データの破棄と新頂点の初期化を含み、所有領域とコピーはO(N+M)です。

## 所有・検査

コピーは独立、move元は0頂点・0辺・0対応、swapはO(1)です。辺追加・reset・コピー代入に失敗しても以前の内容を保ちます。solveは全作業領域を確保してから対応を変更するため、確保失敗時は辺・対応・最大性と既存spanを維持します。

partnersのspanはsolve・clear_matchingで値が更新され、reset・clear・代入・所有権移動で無効になります。edgesのspanは辺追加・予約・clear_edgesでも無効化し得ます。`SKIP_RUNTIME_CHECKS`で負の長さと頂点番号の検査を除き、内部の容量・添字保護は維持します。

[Library Checker General Matching](https://judge.yosupo.jp/problem/general_matching)は単純グラフに対応します。多重辺・再solve・所有・例外保証は独立ローカル検査で補います。


## Document/lca.md

# 最近共通祖先

[LCA](../Graph/LCA.cpp)は無向森を前処理し、最近共通祖先・距離・祖先・パス上の頂点を返します。[使用例](../Examples/lca_example.cpp)を参照してください。距離は辺数です。

## 構築

隣接リスト`std::vector<std::vector<int>>`、または頂点数と無向辺列を渡します。辺列の型は`std::span<const std::pair<int,int>>`です。読み取り専用の配列参照なので、`std::vector<std::pair<int,int>>`をそのまま渡せます。各無向辺を1回だけ記録し、向きは問いません。どちらの入力も構築後には参照を保持しません。

頂点番号は`[0,N)`で、自己ループ・多重辺・閉路を含めません。隣接リストの対称性は利用側の前提です。指定した`root`の成分から処理し、残りは最小未訪問頂点を各成分の根にします。空の入力には`root=0`を指定します。

Nを頂点数、Eを隣接リストの要素数（辺列版では無向辺数の2倍）、B=ceil(N/32)、Hを最大深さとします。構築はO(N+E+B log(B+1))、保持領域はO(N+B log(B+1))、追加作業領域はO(N)です。辺列版はこれに加えてO(N+E)の一時領域で隣接データを作り、構築完了時に解放します。

## API

| API | 動作・戻り値 | 計算量 |
| --- | --- | --- |
| `LCA()` | 空の表を作る。 | O(1) |
| `LCA(g,root=0)` / `build(g,root=0)` | 隣接リストから構築 / 再構築する。 | 構築と同じ |
| `LCA(n,edges,root=0)` / `build(n,edges,root=0)` | 無向辺列から構築 / 再構築する。 | 構築と同じ |
| `size()/empty()/component_count()/root()` | 頂点数・空判定・成分数・指定根。空の根は-1。 | O(1) |
| `parent(v)/depth_of(v)/component_of(v)/root_of(v)` | 親・深さ・成分番号・成分根。根の親は-1。 | O(1) |
| `same_component(u,v)/is_ancestor(u,v)` | 同成分か / uがvの祖先か。自身も含む。 | O(1) |
| `get(u,v)/operator()(u,v)` | 最近共通祖先。異成分なら-1。 | O(1) |
| `get(u,v,new_root)` | new_rootを根としたLCA。3頂点の成分が揃わなければ-1。 | O(1) |
| `get_distance(u,v)` | パスの辺数。異成分なら-1。 | O(1) |
| `kth_ancestor(v,k)` | k個上の祖先。k=0ならv、根を越えると-1。 | O(log(H+2)) |
| `kth_on_path(u,v,k)` | uからvへのパスのk番目（0始まり）。異成分・範囲外なら-1。 | O(log(H+2)) |
| `order_index(v)` | 森全体でのDFS順添字。 | O(1) |
| `subtree_range(v)/subtree_size(v)` | DFS順の部分木半開区間 / 部分木の頂点数。 | O(1) |
| `clear()/swap(other)` | 領域を解放して空にする / 内容を交換する。 | O(N+B log(B+1)) / O(1) |

```cpp
std::vector<std::pair<int,int>> edges{{0,1},{0,2},{1,3},{1,4},{5,6}};
LCA tree(7, edges);                 // 0〜4と5〜6の二成分
int a = tree.get(3, 4);             // 1
int distance = tree.get_distance(3, 2); // 3
int middle = tree.kth_on_path(3, 2, 2); // 0
int disconnected = tree.get(0, 5); // -1
auto [first,last] = tree.subtree_range(1);
tree.build(7, edges, 4);           // 根を変更して再構築
```

## 前提と所有権

照会の頂点番号は表の範囲内で指定します。kは`std::uint64_t`で受け、`UINT64_MAX`も範囲外と判定します。根変更の`get`は答えだけを計算し、部分木の向きは変えません。部分木は構築時の根に対し、`first <= order_index(x) < last`を満たす頂点全体です。兄弟の訪問順は規定しません。

検査有効時は不正な根・頂点・辺列の端点を`out_of_range`、閉路・自己ループ・多重辺を`invalid_argument`で拒否します。辺列版は構築時に全端点を検査します。隣接リストの対称性は検査しません。`SKIP_RUNTIME_CHECKS`で入力検査を除いても、負のnと内部容量超過の`length_error`は維持します。

`build`とコピー代入は失敗時に元の内容を保持します。コピー後の再構築・破棄は互いに影響しません。頂点情報と親表は各オブジェクトが所有し、深い森の不変な照会表は内部共有します。ムーブ元は空で再利用でき、自己コピー・自己ムーブは内容を保持します。

深い森は32要素ブロックのRMQでLCAを求め、Ladder分解で祖先を探索します。浅い森は小さな祖先表を使います。Ladderの数学的参考は[Bender・Farach-Colton](https://www3.cs.stonybrook.edu/~bender/newpub/2004-level.pdf)です。実装の出典ではありません。

提出先は[Library Checker](https://judge.yosupo.jp/problem/lca)です。森・任意の根・追加API・所有権・例外保証は[独立テスト](../Verify/current/tests/lca_reference_test.cpp)で補い、詳しい検証・測定は[Audit](../Audit/)に置きます。


## Document/rerooting_dp.md

# 隣接順を保つ全方位木DP

`RerootingDP<T>`は、森の各頂点を根にした場合のDPをまとめて求めます。例えば「各頂点から全頂点までの距離和」を全ての頂点について計算できます。[本体](../Graph/Rerooting_DP.cpp)・[使用例](../Examples/rerooting_dp_example.cpp)を参照してください。

## 計算の設定

`build(identity,merge,to_vertex,to_edge)`で、辺から受け取る値と頂点の答えを同じ型Tで定義します。

| 引数 | 役割 |
| --- | --- |
| `identity` | メッセージがない場合の値。mergeの左右どちらに渡しても結果を変えない。 |
| `merge(const T&,const T&)->T` | 複数の隣接先から受け取った値を結合する。括り方で結果が変わらないこと。 |
| `to_vertex(const T&,int v)->T` | 結合した値から、頂点vを根とする答えを作る。 |
| `to_edge(const T&,int from,int to,size_t edge_id)->T` | 頂点の値を、fromからtoへ送る値に変える。 |

uからvへ送る値は、v以外からuへ届いた値を結合し、to_vertex、to_edgeの順に変換したものです。頂点uの全体の答えは、全隣接先から届いた値を結合してto_vertexを適用します。結合順は各頂点での**辺の追加順**なので、文字列連結のように順番が重要な演算にも使えます。

```cpp
struct State { std::int64_t sum=0,count=0; };
RerootingDP<State> tree(3);
tree.add_edge(0,1);
tree.add_edge(1,2);
tree.build(State{},
    [](const State& a,const State& b){ return State{a.sum+b.sum,a.count+b.count}; },
    [](const State& a,int){ return State{a.sum,a.count+1}; },
    [](const State& a,int,int,std::size_t){ return State{a.sum+a.count,a.count}; });
// tree[0].sum==3、tree[1].sum==2、tree[2].sum==3。
```

countは頂点数、sumは距離和です。頂点を含めるとcountを1増やし、辺を渡ると各頂点への距離が1ずつ増えるのでsumにcountを加えます。

## API

| API | 動作 |
| --- | --- |
| `RerootingDP<T>(n=0)` / `add_edge(u,v)` | n頂点で構築／無向辺を追加してsize_tの辺IDを返す。 |
| `build(...)` / `values()` / `operator[](v)` | 全根結果のconst spanを返す／同じspan／頂点vの答えのconst参照。 |
| `message(id,reverse=false)` / `messages()` | 追加時u→vの値（trueなら逆向き）／全方向のconst span。 |
| `edge(id)` / `edges()` / `neighbors(v)` | 追加時の端点pair／辺表span／出るarc番号のspan。 |
| `size()` / `empty()` / `edge_count()` / `has_results()` | 頂点数／0頂点か／辺数／現在の結果があるか。各O(1)。 |
| `component_count()` | build後の森の連結成分数。 |
| `reserve_edges(m)` / `reserve_neighbors(v,d)` | 辺表／隣接列を予約する。 |
| `reset(n)` / `clear()` / `swap(other)` | 辺と結果を消してn頂点へ／0頂点へ／交換。 |

各辺idのarcは2*idが追加時のu→v、2*id+1がv→uです。結果取得はhas_results()==trueが前提で、辺を追加したら再buildします。空の森も扱い、孤立点vの答えはto_vertex(identity,v)です。

## 前提・所有と計算量

頂点は0..N-1で、自己辺・多重辺・閉路を認めません。Tはbool・cv修飾型以外でコピー・代入可能、破棄が例外を投げないことが必要です。デフォルト構築は不要です。関数はconst参照を通してbuild中だけ呼び、同じ入力に同じ結果を返すものを使います。実行中に同じグラフを変更しないでください。

値の演算・コピーをO(1)として、buildはO(N+M)時間・追加領域です。探索とDPは反復で行います。結果の参照取得はO(1)、辺追加は償却O(1)、再確保・コピーは保持量に比例します。コピーは独立、move元は空、swapはO(1)です。

build・コピー代入・resetの失敗時は旧内容を保ちます。build失敗時は旧結果の参照も維持します。辺追加に失敗した場合も辺・結果を保ちますが、辺表・隣接列の容量と以前のspanは復元しません。関数の外部状態は巻き戻しません。成功した再構築・reset・clear・代入や、対応配列の再確保で参照は無効化し得ます。

`SKIP_RUNTIME_CHECKS`で頂点・辺番号・結果の有無・森の検査を除き、内部件数・添字の上限は維持します。森の検査は通常の探索に組み込み、検査専用の全辺走査はしません。数学的参考は[Nyaan](https://nyaannyaan.github.io/library/tree/rerooting.hpp.html)、機能上の参考は[Luzhiled](https://ei1333.github.io/library/graph/tree/rerooting.hpp.html)。独立した反復実装を[Library Checker](https://judge.yosupo.jp/problem/tree_path_composite_sum)と各根の素朴な再計算で検証します。


## Document/bipartite_matching.md

# 二部グラフの最大マッチング

`BipartiteMatching(L,R)`は、左L頂点・右R頂点の間で、端点を共有しない辺を最大何本選べるか求めます。左右は別々に0始まりで番号を付けます。[本体](../Graph/Bipartite_Matching.cpp)・[使用例](../Examples/bipartite_matching_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `add_edge(l,r)` | 左lと右rを結ぶ辺を追加する。多重辺も可。 |
| `solve()` | 最大マッチングの総本数を返す。辺追加後も現在の対応から続けて計算する。 |
| `match_left(l)` / `match_right(r)` | 対応する相手の番号。未対応なら-1。 |
| `pairs()` | 左番号順の対応辺をvectorで返す。 |
| `left_matches()` / `right_matches()` | 対応表への読み取り専用spanを返す。 |
| `min_vertex_cover()` / `max_independent_set()` | 最小頂点被覆／最大独立集合を`VertexSet{left,right}`で返す。 |
| `left_size()` / `right_size()` / `edge_count()` | 左頂点数／右頂点数／重複込みの登録辺数。 |
| `matching_size()` / `is_maximum()` | 現在の対応本数／最大と確認済みか。 |
| `neighbors(l)` / `reserve_neighbors(l,n)` | 左lの挿入順の隣接列を参照／容量を予約する。 |
| `clear_matching()` / `clear_edges()` | 辺を残して対応だけ消す／頂点数と容量を残して辺・対応を消す。 |
| `reset(L,R)` / `clear()` / `swap(other)` | 指定頂点数へ初期化／0頂点へ／交換。 |

```cpp
BipartiteMatching graph(3,4);
graph.add_edge(0,2);
graph.add_edge(0,1);
graph.add_edge(1,2);
graph.add_edge(2,0);
auto count = graph.solve(); // 3。
auto pairs = graph.pairs(); // 左右の対応辺。
auto cover = graph.min_vertex_cover(); // cover.size() == 3。
```

## 被覆・独立集合と最大性

頂点被覆は、全ての辺の少なくとも片端を含む頂点集合です。独立集合は、選んだ頂点間に辺がない集合です。結果のleft・rightは左右別の昇順vectorで、size()が合計件数です。被覆の最小件数は最大マッチングの本数に等しく、補集合が最大独立集合になります。

被覆・独立集合の取得は`is_maximum()==true`が前提です。辺追加後はfalseとなるため、先にsolveしてください。初期状態はtrueです。対応表とpairsは最大性未確認でも現在の有効な対応を返します。empty()は左右とも0頂点かを表します。

## 計算量と参照

V=L+R、M=登録辺数として、反復Hopcroft–Karp法でsolveはO((V+M)√(V+1))時間・追加O(L)領域。最大性が既知ならO(1)です。被覆・独立集合はO(V+M)時間・領域、pairsはO(L)時間・O(対応本数)領域です。件数・対応・span取得はO(1)、辺追加は償却O(1)で、再確保時は現在の次数に比例します。

clear_matchingはO(V)、clear_edgesと再構築は旧状態の破棄も含めO(V+M)。コピーはグラフと対応を独立所有し、move元は0頂点、swapはO(1)です。辺追加・コピー代入・resetとsolveの作業領域確保に失敗しても以前の内容を保ちます。

対応表の値はsolveやclear_matchingで更新されます。隣接spanはその左頂点への辺追加・予約で無効化し得ます。reset・clear・代入後は以前のviewを使わないでください。既定では端点と被覆取得時の最大性を検査し、`SKIP_RUNTIME_CHECKS`で除きます。件数・容量の上限保護は維持します。

数学的参考は[Hopcroft–Karpの原論文](https://doi.org/10.1137/0202019)、機能上の参考は[Luzhiled](https://ei1333.github.io/library/graph/flow/bipartite-matching.hpp.html)です。実装は独立に記述しています。[Library Checker](https://judge.yosupo.jp/problem/bipartitematching)と、被覆・独立集合等のローカル検査で検証します。


## Document/two_sat.md

# 2-SAT

`two_sat`は、bool変数に「条件Aまたは条件B」という制約を加え、全制約を同時に満たす値があるか求めます。[本体](../Graph/Two_SAT.cpp)・[使用例](../Examples/two_sat_example.cpp)を参照してください。

型名・コンストラクタ・`add_clause`・`satisfiable`・`answer`の呼び出しと戻り値は[ACL](https://atcoder.github.io/ac-library/production/document_ja/twosat.html)に合わせています。このライブラリでは型をグローバルに置き、`atcoder::`は付けません。

## API

| API | 動作 |
| --- | --- |
| `two_sat()` / `(int n)` | 空／変数0..n-1を用意する。0<=n<=INT_MAX/2。 |
| `add_clause(i,a,j,b)` | `(x_i==a) || (x_j==b)`を追加する。 |
| `imply(i,a,j,b)` | x_i==aならx_j==b、という制約を追加する。 |
| `force(i,a)` | x_iをaに固定する。 |
| `add_xor(i,j,different=true)` | 2変数を異なる値にする。falseなら同じ値にする。 |
| `satisfiable()` | 全制約を満たせるならtrue。複数回実行でき、制約追加後は再判定する。 |
| `answer()` | 解を`std::vector<bool>`の値で返す。常に長さnで、判定前・UNSAT時の内容は未規定。 |
| `value(i)` | 最後の判定がtrueで、以後制約を追加していない場合の変数iのbool値。 |
| `size()` / `clause_count()` / `reserve_clauses(m)` | 変数数／内部の2条件の節数／m節分を予約。 |
| `reset(n=0)` / `swap(other)` | 制約を捨てn変数へ／O(1)交換。 |

```cpp
two_sat constraints(2);
constraints.add_clause(0,true,1,true); // x0かx1の少なくとも一方がtrue。
constraints.force(0,false);
if (constraints.satisfiable()) {
    auto answer = constraints.answer();
    bool x1 = answer[1]; // true。
}
```

`answer()`は独立したコピーなので、返り値を書き換えても内部の解は変わりません。制約を追加した場合は再判定してから新しい解として使います。`value(i)`だけは最後の判定がtrueであることが前提です。XORは内部では2節として数えます。

N変数・M節に対し、構築はO(1)、制約追加は償却O(1)、`satisfiable()`はO(N+M)時間・追加領域です。`answer()`はO(N)時間・領域、`value(i)`はO(1)です。判定中はbyte配列で値を保持し、`answer()`呼び出し時にbit配列へ変換します。

各変数の真偽を2頂点に変換し、強連結成分分解で矛盾を判定します。再帰を使わず、成分の確定時に同じ変数の真偽が同居していないか確認します。別ファイルのincludeに依存せず、SCCと両方を貼り付けても使えます。コピーは独立、move元は空の未解決状態です。

変数番号と`value()`の解の有効性の検査、検査専用状態は`SKIP_RUNTIME_CHECKS`で除きます。内部のサイズ保護と矛盾の数学的判定は維持します。

[独立参照](../Verify/current/tests/new_scc_two_sat_reference_test.cpp)・[Library Checker](https://judge.yosupo.jp/problem/two_sat)


## Document/project_selection.md

# Project Selection

`Graph/Project_Selection.cpp` の `TwoVariableSubmodularOpt<COST>` は、二値変数に対する劣モジュラなコストの総和を最小化する。`true` を最小カットの始点側、`false` を終点側として表す。公開 API はファイル先頭、実行例は `Examples/project_selection_example.cpp` を参照。

## 項と制約

`add_single_cost(i,F,T)` は変数が false/true のとき F/T を加える。`add_psp_penalty(i,j,C)` は `xi=true, xj=false` に非負費用 C を加え、`add_psp_constraint(i,j)` は同じ組を禁止する。`force` と `add_equality` で固定値・等値制約も指定できる。

`add_submodular_function(i,j,A,B,C,D)` は FF/FT/TF/TT のコストを追加する。異なる変数では `B+C >= A+D` が必要。同じ変数には A/D だけを使用する。条件の判定と辺への分解では、加算の前に差を取り、符号付き整数の未定義オーバーフローを避ける。

`add_both_true_profit` / `add_both_false_profit` は二変数がともに true/false の場合の非負利得、`add_all_true_profit(xs,P)` / `add_all_false_profit(xs,P)` は集合全体の非負利得を表す。空集合では利得 P を常に得る。集合内の重複は許される。

`add_any_true_cost(xs,C)` / `add_any_false_cost(xs,C)` は、少なくとも一つが true/false の場合の非負費用 C を表す。空集合の費用は 0。例えば、利用した設備の種類ごとに一度だけかかる費用を、複数の変数をまとめて指定できる。これらは補助頂点で直接表現し、定数加算と利得の二段階操作による一時的な offset の超過を避ける。

```cpp
TwoVariableSubmodularOpt<std::int64_t> model(3);
model.add_single_cost(0, 0, -8); // 選ぶと8の利得
model.add_psp_constraint(0, 1);  // 0を選ぶなら1も選ぶ
model.add_any_true_cost({0, 1}, 3);
model.add_any_false_cost({0, 1, 2}, 2);
auto [cost, assignment] = model.solve_with_assignment();
```

集合項の費用・利得が 0 の場合は無操作で、未参照の添字は検査しない。`SKIP_RUNTIME_CHECKS` を定義すると、添字・非負性・劣モジュラ性の入力検査を除く。変数数・内部添字・容量予算・算術の上限は省略時も維持する。不正入力を未参照部分まで一括検査する API ではない。

## 数値と解法

`COST` は 64 bit 以下の符号付き整数型。入力値だけでなく、分解途中の符号付きコスト・offset・最適値もこの型の範囲に収まる必要がある。有限辺の容量総和は `std::make_unsigned_t<COST>` の最大値未満に制限する。差は符号なし演算で正確に表し、厳密制約の容量にはその最大値を使用する。有限容量の総和を超えるフローが流れたら制約矛盾として `std::domain_error` を送出する。容量・数値の超過は `std::overflow_error`。旧コンストラクタの `inf` 引数は互換性のために受け付け、値は使用しない。

残余網を隣接配列で保持し、Dinic 法で解く。再帰は固定の深さまでに制限し、それより長い増加路は明示的なスタックで処理する。同じ層グラフで使い切った頂点を無効にし、再訪を減らす。補助頂点込みで65536頂点以上なら、BFSキューに残る訪問頂点を使って層と探索位置を初期化する。訪問が全頂点の4分の1以下なら該当する添字だけを戻し、多ければ連続領域を一括で埋める。終点の層は個別に戻す。層配列の新規領域は規模にかかわらず未訪問で初期化し、確保失敗後の再試行でも訪問済み頂点をキューから復元できるようにする。頂点追加後の再開・最小カットの復元にも同じ残余網を使い、訪問管理用の追加配列は持たない。標準 C++23 だけで実装し、専用の SIMD・128 bit 整数経路は持たない。

補助頂点込みの頂点数を V、辺数を E とすると、解法は `O(V²E)`、保存領域は `O(V+E)`。単一項の追加は償却 `O(1)`、集合項は `O(|xs|)`。`solve()` は変更がなければ `O(1)` で、項の追加後は残余網から再開する。定数の追加はフローを再計算しない。`reconstruct()` は必要に応じて解いてから、追加 `O(n)` で最適割当を返す。同点時の割当は固定しない。

## 再利用・所有権・例外

`clear()` は変数数を維持して項と解を消去する。補助頂点を取り除き、残る頂点の辺配列と作業配列の容量を再利用する。確保を行わず `noexcept`、計算量は `O(V+E)`。`init(n)` は隣接配列を作り直すため、旧隣接配列の領域を解放する。作業配列の容量は保持する。`reserve_nodes(count)` は頂点配列の予約で、辺配列の予約ではない。

コピーは独立した所有、移動後の元は空の 0 変数モデル。`empty()` は変数数が 0 かを返す。メンバ `swap` と ADL の `swap(a,b)` は定数時間で所有権を交換する。

項の追加・`init`・コピー代入が失敗しても、変数数・追加済みの項・最適値は変えない。ただし確保済み容量は変わり得る。複数辺を追加する操作は先に必要な容量を予約し、集合項は失敗時に追加分だけ巻き戻す。`solve()` の確保失敗後も再試行できる。制約矛盾は項追加では解消できないので `clear` / `init` で作り直す。最適値の型超過はフロー計算後に判定し、定数を調整して収まる場合は再取得できる。

## 検証と資料

Verify は [AtCoder 典型90 040](https://atcoder.jp/contests/typical90/tasks/typical90_an)。main は入力を単一項・含意制約に変換して解くだけとする。公開サンプルに加え、全割当列挙と独立な行列形式の Edmonds–Karp 法を使った補足入力を生成する。この問題で扱わない集合項などは、`Verify/current/tests/project_selection_reference_test.cpp` の独立エネルギー表・境界試験で照合する。深い分岐・合流・逆辺による経路変更は `project_selection_deep_flow_test.cpp` で独立な最大流と照合する。

アルゴリズムの参考は [drken の解説](https://drken1215.hatenablog.com/entry/2023/11/24/034300)。由来とライセンス、測定条件・不採用案、検証結果は `Audit/` の Project Selection 関連記録を参照。ローカル検証とオンライン提出の状態は区別する。

