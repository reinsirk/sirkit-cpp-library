# 出次数の偶奇を指定する辺の向き付け

無向グラフの各辺に向きを付け、頂点ごとの出次数を指定した偶奇にそろえます。[本体](../Graph/Parity_Orientation.cpp)

- `orient_with_outdegree_parity(n, edges, required = {})` の辺は $0$ 始まりの端点の組です。頂点 `v` で偶数にするなら `required[v] = 0`、奇数にするなら `1` を指定します。`required` を省くと全頂点を偶数にします。
- 辺の入力順に `(出発点, 到着点)` を返します。実現できなければ `std::nullopt` です。非連結グラフ、多重辺、自己ループにも対応し、自己ループはその頂点の出次数を 1 増やします。

全域森以外の辺の向きを先に決め、葉から根に向かって残りの辺を調整します。各連結成分で辺数と指定した奇数頂点の数の偶奇が一致するときに限り、向き付けが可能です。時間・領域は $O(n+m)$ です。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
const auto result = sirkit::orient_with_outdegree_parity(3, edges);
// 向きは 1 → 0 と 1 → 2。
```

[使用例](../Examples/parity_orientation_example.cpp)・[Verify](../Verify/current/submissions/parity_orientation.cpp)
