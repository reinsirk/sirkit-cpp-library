# 有向グラフ上のゲームを後退解析する

2人が交互に有向辺をたどり、動けなくなった側が負けるゲームを解きます。同じ頂点や辺を何度でも使えます。[本体](../Graph/Directed_Game.cpp)

`solve_directed_game(n, edges)` は $0$ 始まりの端点の組を受け取り、各頂点に対して手番側の結果を返します。

- `outcome` は `DirectedGameOutcome::win`、`lose`、`draw` のいずれかです。
- `plies` は勝つ側が早く終わらせ、負ける側が長く粘る場合の残り手数です。引き分けなら `-1` です。
- `next` はその手数を達成する行先です。引き分けでは負けを避けられる行先を返し、動けない頂点では `-1` になります。

動けない頂点を負けとして確定し、辺を逆向きにたどって結果を伝えます。勝ちも負けも確定しない頂点は引き分けです。時間・領域は $O(n+m)$ です。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{2, 2}};
const auto positions = sirkit::solve_directed_game(3, edges);
// 0 では勝ち、1 では負け、2 では引き分け。
```

[使用例](../Examples/directed_game_example.cpp)・[Verify](../Verify/current/submissions/directed_game.cpp)
