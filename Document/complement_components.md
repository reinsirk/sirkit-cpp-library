# 補グラフの連結成分

元の無向グラフで結ばれていない頂点対を辺とみなし、その連結成分を求めます。

```cpp
std::vector<std::pair<int, int>> edges{{0, 1}};
auto components = sirkit::complement_components(3, edges);
```

## `complement_components`

```text
std::vector<std::vector<int>> complement_components(int n, std::span<const std::pair<int, int>> edges);
```

頂点番号は `[0,n)` です。自己辺・平行辺は結果に影響しません。成分と頂点の並び順は任意です。補辺を作らず、元の辺数を $m$ として $O(n+m)$ 時間・領域で計算します。

[使用例](../Examples/complement_components_example.cpp) ／ [Verify](../Verify/current/submissions/complement_components.cpp)
