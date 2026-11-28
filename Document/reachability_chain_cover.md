# 到達可能性で分ける最小チェーン被覆

`minimum_reachability_chain_cover(n, edges)` は有向グラフの全頂点を最少個のチェーンに分けます。各チェーンでは前の頂点から後の頂点へ到達でき、途中で別のチェーンの頂点を通っても構いません。[本体](../Graph/Reachability_Chain_Cover.cpp)

頂点は $0$ 始まり、辺は `(出発点, 到着点)` です。自己ループ、多重辺、閉路があるグラフにも使えます。各頂点は結果のちょうど1つのチェーンに現れます。

強連結成分を縮約し、別のチェーンが同じ成分を経由できる最大流を1回求めます。強連結成分数を $c$、縮約後の辺数を $a$ として、最大流は $O(c+a)$ 規模のグラフで処理します。前後処理は最大流を除いて $O(n+m+L)$ 時間、全体で $O(n+m)$ 領域です。$L$ は復元する経路の長さの合計で、高々 $c^2$ です。

## 使用例

```cpp
const std::array edges{std::pair{0, 2}, std::pair{1, 2}, std::pair{2, 3}, std::pair{2, 4}};
const auto chains = sirkit::minimum_reachability_chain_cover(5, edges);
// 2チェーン。一例は {0, 2, 3} と {1, 4}。1 から 4 へは 2 を通れる。
```

[使用例](../Examples/reachability_chain_cover_example.cpp)・[Verify](../Verify/current/submissions/reachability_chain_cover.cpp)・[商品名のVerify](../Verify/current/submissions/product_name_chain_cover.cpp)
