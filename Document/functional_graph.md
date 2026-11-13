# FunctionalGraph

[本体](../Graph/Functional_Graph.cpp) / [使用例](../Examples/functional_graph_example.cpp)

各頂点から出る辺が1本の有向グラフを分解する。頂点は `0..n-1`、行先のラベル順に制約はない。自己辺、複数の周期、周期へ合流する木を扱う。各弱連結成分は1つの有向周期を持つ。

```cpp
std::vector<int> next{2, 0, 3, 2};
FunctionalGraph graph(next);
int v = graph.kth_successor(1, 1000000000000000000ULL);
auto distance = graph.distance(1, 3); // 3
```

`FunctionalGraph()` は空。`FunctionalGraph(std::span<const int> successor)` / `build(successor)` は行先をコピーして構築する。空配列も使える。再構築の入力として自身の `successors()` を渡してよい。

`kth_successor(v,k)` はk本の辺を進んだ頂点を返す。k=0はv自身で、`std::uint64_t` の最大値まで対応する。木の深さより小さい歩数だけダブリングを使い、周期に到達する場合は歩数から深さを引いて周期長の剰余を取る。周期用に64段の表を持つ必要はない。

`distance(from,to)` はfromからtoへ初めて到達する辺数で、到達不能は-1。自分自身は0。`reachable(from,to)` はその到達可否。同じ弱連結成分でも、枝の異なる木の頂点へは到達できない。行先が木の頂点なら深さの差だけ進んで一致を調べ、周期頂点なら周期内の向き付き距離を足す。

`info(v)` は `CycleInfo {id,entry,depth,position,length}` を返す。idは最終的に入る周期ID、entryは最初に到達する周期頂点、depthはそこまでの辺数、positionはentryの周期内添字、lengthは周期長。`cycle(id)` はsuccessor方向に並ぶ周期頂点の `std::span<const int>`。IDと周期の始点は内部の構築順で、外部の特定ラベルと対応付けない。周期上の頂点はdepth=0、entry=vとなる。

`size()` / `empty()`、`successor(v)` / `successors()`、`on_cycle(v)`、`cycle_count()`、`component_size(v)` も使える。`component_size(v)` はvと同じ弱連結成分全体の頂点数で、周期長とは異なる。空グラフの周期数は0。

入次数0の頂点をキューで取り除き、残った頂点から周期を列挙する。取り除いた順を逆にたどって各頂点の周期情報と深さを決める。全処理は反復式で、長鎖にも再帰を使わない。構築は O(n log(n+1)) 時間・領域。`kth_successor`、`distance`、`reachable` は O(log(n+1))、その他の照会は O(1)。

コピーは全配列を独立所有する。copy代入と `build` は、確保失敗時に旧状態と旧viewを維持する。move / swapはO(1)で、move元は空となり再利用できる。`clear()` は空へ戻す。viewは成功した再構築・clear・代入・所有権移動まで有効。頂点と周期IDの検査は既定で有効で、行先の検査は構築に必要な入力走査の中で行う。`SKIP_RUNTIME_CHECKS` 定義時は検査を除き、有効な番号を前提とする。配列容量の上限保護は残す。

Library Checkerで直接対応する周期・到達距離問題を確認できなかったため、Verifyには [AtCoder ABC367 E - Permute K times](https://atcoder.jp/contests/abc367/tasks/abc367_e) を使う。k回先の取得を実問題で扱い、到達距離・周期情報・全uint64_t範囲・所有と例外保証は独立参照で補足する。
