# トポロジカルソート

各辺`u→v`でuがvより先となる頂点順を求めます。閉路があるグラフでは作れません。[使用例](../Examples/topological_example.cpp)と[辞書順最小の例](../Examples/topological_lexicographical_example.cpp)を参照してください。

`Graph/Topological.cpp`と`Graph/Topological_sort.cpp`は同じ公開API・共有ガードを持ち、どちらか一方を貼り付けて使えます。Graphは`std::vector<std::vector<int>>`で、graph[u]にuからの行先を入れます。

## API

### TopologicalSort

```text
inline std::vector<int> TopologicalSort(const Graph& graph);
```

入次数0の頂点をFIFOで処理した順序を返します。閉路があれば空列です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(V+E)時間・O(V)領域。

### TopologicalSortLexicographical

```text
inline std::vector<int> TopologicalSortLexicographical(const Graph& graph);
```

条件を満たす順序のうち辞書順最小のものを返します。閉路があれば空列です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(E+V log(V+1))時間・O(V)領域。

### TopologicalSortDetails

```text
inline TopologicalResult TopologicalSortDetails(const Graph& graph, bool lexicographical = false);
```

頂点の順序、各頂点の位置、DAGかどうか、順序が一意かどうかを返します。lexicographicalがtrueなら辞書順です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** lexicographical=falseならO(V+E)、trueならO(E+V log(V+1))時間。O(V)領域。

### IsDAG

```text
inline bool IsDAG(const Graph& graph);
```

有向閉路がなければtrueを返します。空グラフでもtrueです。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(V+E)時間・O(V)領域。


Detailsは`TopologicalResult`を返します。orderは頂点列、position[v]は列中の位置です。acyclicは閉路がないか、uniqueは順序が一意かを表します。空グラフでは順序が空になり、acyclicとuniqueはtrueです。

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

FIFO版はO(V+E)時間、辞書順版はO(E+V log(V+1))時間、追加領域はどちらもO(V)です。Detailsは選んだ順序の方式と同じ計算量です。入力は変更せず、戻り値の配列は独立して所有します。

頂点数はINT_MAX以下、各入次数はUINT32_MAX以下、辺の終点は`[0,V)`が前提です。終点の検査は通常の入次数集計中に行い、`SKIP_RUNTIME_CHECKS`で省きます。容量の保護は検査省略時も維持します。

SIMDの指定は[共通設定](simd.md)、内部処理・出典・検証は記録を参照してください。
