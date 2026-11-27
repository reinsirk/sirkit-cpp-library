# 最遠点対

整数座標の点列から、距離が最大の2点の添字と距離の二乗を求めます。添字は元の入力順に対応します。

```cpp
std::vector<std::pair<std::int32_t, std::int32_t>> points{{0, 0}, {3, 4}, {1, 1}};
auto pair = sirkit::furthest_pair(points);
assert(pair->distance_squared == 25);
```

## `furthest_pair`

```text
std::optional<FurthestPairResult>
furthest_pair(std::span<const std::pair<std::int32_t, std::int32_t>> points);
```

戻り値の `first_index` と `second_index` は0始まりで、`first_index < second_index` です。同じ最大距離を持つ対が複数あれば、そのうちの1つを返します。同じ座標の点を複数渡せます。

2点未満では `std::nullopt` を返します。最大距離の二乗が `std::uint64_t` の上限を超えると、`std::overflow_error` を送出します。

凸包を求め、回転キャリパー法で対向する頂点を調べます。$N$ 点で $O(N\log(N+1))$ 時間・$O(N)$ 追加領域です。

[使用例](../Examples/furthest_pair_example.cpp) ／ [Verify](../Verify/current/submissions/furthest_pair.cpp)
