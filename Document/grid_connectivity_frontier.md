# グリッドの境界連結性

グリッドを左の列から、各列の上から下へ処理します。`GridConnectivityFrontier(h)` は、処理済みの黒マスが境界でどうつながっているかを保持します。高さは `1..14` です。

| API | 内容 |
| --- | --- |
| `place(state, row, black, mark = false)` | 1マス処理して新しい `state` と、境界から消えた成分の `closed_group` を返す。消えていなければ `0`。$O(h)$。 |
| `label(state, row)` | 白なら `0`、黒なら成分番号。印のある成分は `kMarkedGroup`。$O(1)$。 |

空の境界は `state = 0` です。黒マスに印を付けると、その成分が併合されても印は引き継がれます。同時に印を付けられる成分は1つです。成分番号は各状態で振り直すため、別の状態にある普通の成分どうしの比較には使えません。

```cpp
auto ways = sirkit::grid_corner_connected_colorings(2, 2); // 3
```

`grid_corner_connected_colorings(h, w, mod)` は、左上と右下を結ぶ黒マスの塗り方を数えます。短辺を高さ $H$、長辺を $W$、途中の状態数の最大を $S$ とすると、期待 $O(H^2WS)$ 時間、$O(S)$ 領域です。

[使用例](../Examples/grid_connectivity_frontier_example.cpp) ／ [Verify](../Verify/current/submissions/grid_corner_connected_colorings.cpp) ／ [同じ用途のLuzhiledライブラリ](https://ei1333.github.io/library/other/connected-grid-states.hpp)
