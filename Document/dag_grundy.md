# DAG 上のゲームの Grundy 数

各頂点を局面とし、手番ごとに1本の有向辺をたどる不偏ゲームを扱います。合法手がない側が負けです。[本体](../Graph/DAG_Grundy.cpp)

`dag_grundy_numbers(moves)` に $0$ 始まりの移動先を並べた隣接リストを渡すと、頂点ごとの Grundy 数が得られます。動けない頂点の値は $0$ です。同じ移動先が重複していても構いません。閉路があれば `std::nullopt` を返します。

各手番で独立したゲームのうち1つを進めるなら、現在局面の Grundy 数の XOR が $0$ の状態は先手負けです。各頂点で移動先が持たない最小の非負整数を逆トポロジカル順に求めます。$O(n+m)$ 時間、追加 $O(n)$ 領域です。[閉路も許す勝敗判定](directed_game.md)

## 使用例

```cpp
const sirkit::Graph moves{{}, {0}, {0, 1}};
const auto grundy = sirkit::dag_grundy_numbers(moves);
// 順に 0、1、2。頂点1と頂点2のゲームを合わせた XOR は3。
```

[使用例](../Examples/dag_grundy_example.cpp)・[Verify](../Verify/current/submissions/dag_grundy.cpp)
