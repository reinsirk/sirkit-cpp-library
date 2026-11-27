# クリーク列挙

単純無向グラフで、選んだ頂点がすべて互いに隣接する集合を列挙します。孤立頂点も1頂点のクリークとして数えます。

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
std::size_t count = 0;
sirkit::enumerate_cliques(3, edges, [&](std::span<const int>) { ++count; });
assert(count == 5);
```

## `enumerate_cliques`

```text
template <class Visit>
void enumerate_cliques(int n, std::span<const std::pair<int, int>> edges, Visit &&visit);
```

頂点番号は `[0,n)` です。自己辺や重複辺を含まない入力を指定します。非空のクリークに対して `visit(std::span<const int>)` を1回ずつ呼びます。列挙順と各集合の頂点順は任意です。渡された `span` はコールバックの実行中だけ有効です。

各頂点から後ろに向かう辺を $d$ 本以下に抑えられる頂点順序を使います。そのような順序が存在する最小の $d$ を退化度と呼びます。頂点数を $n$、辺数を $m$、非空クリーク数を $C$ とすると、コールバックを除いて $O(n+m+(d+1)C)$ 時間・$O(n+m)$ 領域です。

[使用例](../Examples/enumerate_cliques_example.cpp) ／ [Verify](../Verify/current/submissions/enumerate_cliques.cpp) ／ [コア分解の原論文](https://arxiv.org/abs/cs/0310049)
