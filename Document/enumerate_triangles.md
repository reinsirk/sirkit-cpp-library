# グラフの三角形列挙

単純無向グラフの三角形を列挙します。三角形の頂点を、その場で集計できます。

```cpp
std::vector<std::pair<int, int>> edges{{0, 1}, {0, 2}, {1, 2}};
int count = 0;
sirkit::enumerate_triangles(3, edges, [&](int, int, int) { ++count; });
```

## `enumerate_triangles`

```text
template <class Visit>
void enumerate_triangles(int n, std::span<const std::pair<int, int>> edges, Visit &&visit);
```

三角形ごとに `visit(u, v, w)` を1回呼びます。列挙順と各三角形の頂点順は任意です。自己辺と重複辺を除いた辺列を渡します。呼び出し先の処理を除き、$O(n+m\sqrt{m})$ 時間・$O(n+m)$ 領域です。

[使用例](../Examples/enumerate_triangles_example.cpp) ／ [Verify](../Verify/current/submissions/enumerate_triangles.cpp)
