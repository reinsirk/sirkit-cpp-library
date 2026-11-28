# 無向辺を向き付けて橋以外を強連結にする

`orient_bridge_blocks(n, edges)` は無向辺に一方向を割り当てます。橋を除いてつながっていた頂点は、向き付けた後も互いに行き来できます。[本体](../Graph/Bridge_Block_Orientation.cpp)

頂点は $0$ 始まりで、辺は端点の組を渡します。結果は入力順の `(出発点, 到着点)` です。自己ループ、多重辺、複数の連結成分にも対応します。

DFS 木の辺は根から離れる向き、その他の辺は祖先に向かう向きにします。橋は片方向にしか通れないため、結果の強連結成分は元のグラフの二辺連結成分と一致します。時間・領域は $O(n+m)$ です。

## 使用例

```cpp
const std::vector edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}, std::pair{2, 3}};
const auto directed = sirkit::orient_bridge_blocks(4, edges);
// 0、1、2 は互いに到達でき、辺 2 → 3 を使って 3 にも行ける。
```

[使用例](../Examples/bridge_block_orientation_example.cpp)・[Verify](../Verify/current/submissions/bridge_block_orientation.cpp)
