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
