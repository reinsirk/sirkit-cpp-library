# 最大独立集合

無向グラフから、選んだ頂点同士が隣接しないようにできるだけ多くの頂点を選びます。重み版では頂点の重みの合計を最大化します。

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
const auto vertices = sirkit::maximum_independent_set(3, edges);  // {0, 2}

const std::array<std::int64_t, 3> weights{2, 5, 2};
const auto result = sirkit::maximum_weight_independent_set(weights, edges);
// result.weight == 5, result.vertices == {1}
```

## `maximum_independent_set`

```text
std::vector<int> maximum_independent_set(int n, std::span<const std::pair<int, int>> edges);
```

最大個数の独立集合を、頂点番号の昇順で返します。頂点数は0〜46、端点の番号は `[0,n)` です。多重辺を扱え、自己辺のある頂点は選びません。頂点数 $n$、辺数 $m$ に対して $O(n+m+2^{\lceil n/2\rceil})$ 時間、$O(n+2^{\lfloor n/2\rfloor})$ 領域です。

## `maximum_weight_independent_set`

```text
MaximumWeightIndependentSetResult maximum_weight_independent_set(
    std::span<const std::int64_t> weights, std::span<const std::pair<int, int>> edges);
```

`weights` の長さを頂点数とし、`weight` に最大の合計、`vertices` に選んだ頂点を昇順で返します。重みが0以下の頂点は選びません。最適値が `std::int64_t` を超えると `std::overflow_error` を送出します。グラフの条件と計算量は `maximum_independent_set` と共通です。

[使用例](../Examples/maximum_independent_set_example.cpp) ／ [Verify](../Verify/current/submissions/maximum_independent_set.cpp)
