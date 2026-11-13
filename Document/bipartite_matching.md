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
