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
