# 各辺を通る長さ4の閉路の数

各無向辺について、その辺を使う長さ4の単純閉路を数えます。平行辺は入力番号で区別します。

```cpp
const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}, {2, 3}, {3, 0}, {3, 0}};
auto count = sirkit::count_four_cycles_per_edge(4, edges);  // {2, 2, 2, 1, 1}
```

## `count_four_cycles_per_edge`

```text
std::vector<std::uint64_t> count_four_cycles_per_edge(
    int n, std::span<const std::pair<int, int>> edges);
```

答えは入力辺と同じ順序です。頂点番号は `[0,n)`、辺数 $m$ は400万までです。閉路は異なる4頂点を通り、始点や向きの違いは区別しません。自己辺の答えは0です。自己辺・重複を除いた辺数を $d$ として $O(n+m\log(m+1)+d\sqrt{d})$ 時間、$O(n+m)$ 領域です。

[使用例](../Examples/count_four_cycles_example.cpp) ／ [Verify](../Verify/current/submissions/counting_c4.cpp)
