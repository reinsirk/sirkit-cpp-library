# 閉路検出

グラフに閉路があれば、辺番号を含めて閉路を1つ求めます。有向・無向の多重グラフに対応します。

```cpp
std::vector<std::pair<int, int>> edges{{0, 1}, {1, 0}};
auto cycle = sirkit::find_cycle(2, edges);
```

## `find_cycle`

```text
std::optional<Cycle> find_cycle(int n, std::span<const std::pair<int, int>> edges, bool directed = true);
```

頂点番号は `[0,n)` です。戻り値の `vertices[i]` とその次の頂点を、入力位置 `edge_ids[i]` の辺が結びます。閉路がなければ `std::nullopt` を返します。$m$ 辺のとき時間・領域ともに $O(n+m)$ です。

[使用例](../Examples/cycle_detection_example.cpp) ／ [Verify](../Verify/current/submissions/cycle_detection_directed.cpp)
