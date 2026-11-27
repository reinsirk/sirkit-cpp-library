# 凸多角形の各頂点からの最遠点

狭義凸多角形の各頂点について、距離が最大になる相手の添字を求めます。

```cpp
using Point = std::pair<std::int64_t, std::int64_t>;
std::vector<Point> square{{0, 0}, {4, 0}, {4, 4}, {0, 4}};
auto farthest = sirkit::all_furthest_neighbors(square);  // {2, 3, 0, 1}
```

## `all_furthest_neighbors`

```text
std::vector<int> all_furthest_neighbors(std::span<const std::pair<std::int64_t, std::int64_t>> vertices);
```

頂点を時計回りか反時計回りの境界順に渡します。座標は `std::pair<std::int64_t, std::int64_t>` の `{x, y}` です。3点以上では、相異なる頂点からなる狭義凸多角形を指定します。戻り値は入力順の `std::vector<int>` で、添字は0始まりです。同距離の最遠点が複数ある場合はそのうち1点を返します。

0点では空、1点では `{0}`、2点では `{1, 0}` です。各軸の座標幅（最大値と最小値の差）の上限は $2\times10^9$ で、超過時は `std::overflow_error` を送出します。通常版では3点以上で狭義凸多角形の条件を満たさない場合に `std::invalid_argument` を送出します。循環展開した列数 $2N-1$ が `int` の上限を超える場合は `std::length_error` です。

循環順に広げた距離行列を補完し、全単調性を使ってSMAWKで各行の最大値を探します。点数を $N$ とすると $O(N)$ 時間・$O(N)$ 領域です。

`smawk` などの公開 API を同梱しています。 [行最小値探索と min-plus 畳み込み](monge_optimization.md)

[使用例](../Examples/all_furthest_neighbors_example.cpp) ／ [Verify](../Verify/current/submissions/all_furthest_neighbors.cpp) ／ [原論文](https://doi.org/10.1007/BF01840359)
