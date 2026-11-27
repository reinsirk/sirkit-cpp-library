# 全域木の数え上げ

行列木定理で、無向グラフと根付き有向グラフの全域木を数えます。

```cpp
using Mint = atcoder::modint998244353;
using Edge = sirkit::SpanningTreeCountEdge<Mint>;
const std::array edges{Edge{0, 1}, Edge{1, 2}, Edge{0, 2}};
const auto undirected = sirkit::count_undirected_spanning_trees<Mint>(3, edges);  // 3
const auto directed = sirkit::count_directed_spanning_trees<Mint>(3, 0, edges);  // 2
```

## `count_undirected_spanning_trees`

```text
template <class Mint>
Mint count_undirected_spanning_trees(int n, std::span<const SpanningTreeCountEdge<Mint>> edges);
```

無向全域木を数えます。頂点数は1以上で、頂点番号は0始まりです。多重辺は区別し、自己辺は除きます。1頂点なら空の木が1通りです。

`SpanningTreeCountEdge<Mint>{from, to, weight}` を渡します。各全域木の辺の重みを掛け、その積をすべての全域木にわたって足します。重みの既定値は1です。`Mint` は素数を法とする modint などの正確な体の型で、演算は $O(1)$ を仮定します。頂点数 $n$、辺数 $m$ に対して $O(n^3+m)$ 時間、$O(n^2)$ 領域です。

## `count_directed_spanning_trees`

```text
template <class Mint>
Mint count_directed_spanning_trees(
    int n, int root, std::span<const SpanningTreeCountEdge<Mint>> edges);
```

`from → to` の向きに辺をたどって根 `root` から全頂点に到達する有向全域木を数えます。入力の制約、重みの扱い、計算量は `count_undirected_spanning_trees` と共通です。

## `count_eulerian_circuits`

```text
template <class Mint>
Mint count_eulerian_circuits(int n, std::span<const std::pair<int, int>> edges);
```

全辺を1回ずつ通る有向閉路を、開始位置の違いを同一視して数えます。自己辺と平行辺は入力番号で区別し、孤立頂点は除きます。辺がなければ1、条件を満たす閉路がなければ0です。

`Mint` は非負整数から構築できる体の型を指定します。頂点数 $n$、辺数 $m$、辺に接する頂点数 $k$ に対して、$O(n+m+k^3)$ 時間・$O(n+m+k^2)$ 領域です。 [Verify](../Verify/current/submissions/counting_eulerian_circuits.cpp)

[使用例](../Examples/counting_spanning_trees_example.cpp) ／ [無向版のVerify](../Verify/current/submissions/counting_spanning_tree_undirected.cpp) ／ [有向版のVerify](../Verify/current/submissions/counting_spanning_tree_directed.cpp) ／ [行列木定理の参考文献](https://doi.org/10.1137/0603033)
