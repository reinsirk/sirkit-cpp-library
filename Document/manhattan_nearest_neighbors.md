# マンハッタン距離の最近点

平面上の各点から、マンハッタン距離 `|x₁-x₂| + |y₁-y₂|` が最小になる相手を探します。

```cpp
const std::vector<sirkit::geo::ManhattanPoint> points{{0, 0}, {3, 1}, {2, 1}};
const auto nearest = sirkit::geo::manhattan_nearest_neighbors(points);
nearest[0]->distance; // 3
nearest[0]->index;    // 2
```

[使用例](../Examples/manhattan_nearest_neighbors_example.cpp)

## `manhattan_nearest_neighbors`

```text
std::vector<std::optional<ManhattanNeighbor>>
manhattan_nearest_neighbors(std::span<const ManhattanPoint> points)
```

`ManhattanPoint` は `std::array<std::int32_t, 2>` です。入力順に結果を返します。各結果の `distance` は距離、`index` は相手の0始まりの添字です。同じ距離なら添字が最小の相手を選びます。同じ座標に別の点があれば距離は0です。

点が1つのときは `std::nullopt` を入れた配列を返し、空の入力には空の配列を返します。点数を `n` とすると `O(n log(n+1))` 時間、`O(n)` 領域です。
