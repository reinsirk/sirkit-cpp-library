# 関数グラフの経路積

各頂点から出る辺が1本のグラフで、指定回数だけ進んだ頂点と、通った辺のラベルの積を求めます。和や最大値に加えて、文字列の連結や関数合成など順序がある演算にも使えます。[本体](../Graph/Functional_Graph_Fold.cpp)

```text
FunctionalGraphFold<T, Combine>(next, label, identity, combine = {});
walk(v, k) -> std::pair<int, T>;
size() -> int;
```

- `next[v]` が頂点 `v` の行き先、`label[v]` がその辺のラベルです。頂点番号は $0$ 始まりです。
- `combine(a, b)` は先に通った区間の値 `a` と次の区間の値 `b` をこの順に結合します。結合則を満たし、`identity` は両側の単位元とします。既定の演算は `std::plus<T>` です。
- `walk(v, k)` は進んだ先と積を返します。回数 `k` は `std::uint64_t` で、$0$ 回なら `{v, identity}` を返します。
- `size()` は頂点数を返します。

演算と値のコピーを $O(1)$ とすると、前処理と領域は $O(n\log(n+1))$、`walk` は $O(\log(n+1)+\log(k+1))$ です。周期を何周もする場合は、1周分の積を繰り返し二乗法でまとめます。

## 使用例

```cpp
const std::array next{1, 2, 1};
const std::array<std::uint64_t, 3> label{10, 2, 5};
const sirkit::FunctionalGraphFold<std::uint64_t> walk(next, label, 0);
const auto [v, sum] = walk.walk(0, 5);  // v == 1, sum == 24
```

[使用例](../Examples/functional_graph_fold_example.cpp)・[和のVerify](../Verify/current/submissions/functional_graph_fold.cpp)・[最大接頭辞和のVerify](../Verify/current/submissions/functional_graph_fold_max_prefix.cpp)
