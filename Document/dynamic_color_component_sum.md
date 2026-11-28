# 木の同色連結成分の重み和

固定した無向木で頂点の色や重みを更新し、指定した頂点と同じ色の頂点だけでたどれる連結成分の重みを合計します。

## `DynamicColorComponentSum`

```text
DynamicColorComponentSum(std::span<const std::int64_t> weights,
                         std::span<const int> colors,
                         std::span<const Edge> edges)
```

頂点番号は0始まりで、色は0または1です。`Edge` は両端の番号を持つ `std::pair<int, int>` です。各頂点の重みと色、木の各辺を1回ずつ渡します。空の木も構築できます。頂点数を $N$ とすると、構築は $O(N+1)$ 時間、領域は $O(N+1)$ です。

入力検査が有効な場合、範囲外の頂点番号には `std::out_of_range`、色や辺の条件を満たさない入力には `std::invalid_argument` を送出します。

重みの加算と成分の和は $2^{64}$ を法として計算し、符号付き64ビット整数で返します。正確な値が `std::int64_t` に収まる場合はその値を得られます。

## `size`

```text
int size() const noexcept
```

頂点数を $O(1)$ で返します。

## `color`

```text
int color(int v) const
```

頂点 `v` の色を $O(1)$ で返します。

## `weight`

```text
std::int64_t weight(int v) const
```

頂点 `v` の重みを $O(1)$ で返します。

## `set_color`

```text
void set_color(int v, int color)
```

頂点 `v` の色を0または1に設定します。$O(\log^2(N+1))$ 時間です。

## `toggle`

```text
int toggle(int v)
```

頂点 `v` の色を反転し、新しい色を返します。$O(\log^2(N+1))$ 時間です。

## `set_weight`

```text
void set_weight(int v, std::int64_t weight)
```

頂点 `v` の重みを置き換えます。$O(\log^2(N+1))$ 時間です。

## `add_weight`

```text
void add_weight(int v, std::int64_t delta)
```

頂点 `v` の重みに `delta` を加えます。負の値も渡せます。$O(\log^2(N+1))$ 時間です。

## `component_sum`

```text
std::int64_t component_sum(int v) const
```

頂点 `v` を含む同色連結成分の重みの和を $O(\log^2(N+1))$ で返します。

## 使用例

```cpp
const std::array<std::int64_t, 3> weights{4, 6, 2};
const std::array colors{0, 1, 0};
const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
sirkit::DynamicColorComponentSum tree(weights, colors, edges);
assert(tree.component_sum(0) == 4);
tree.toggle(1);
assert(tree.component_sum(0) == 12);
tree.add_weight(2, 5);
assert(tree.component_sum(1) == 17);
```

[使用例](../Examples/dynamic_color_component_sum_example.cpp) ／ [Verify](../Verify/current/submissions/dynamic_color_component_sum.cpp) ／ [重軽分解](heavy_light_decomposition.md) ／ [Fenwick木](fenwick_tree.md)
