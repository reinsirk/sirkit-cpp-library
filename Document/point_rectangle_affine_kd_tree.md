# 点代入・長方形アフィン更新・和

後から追加する点も含め、座標を最初に登録します。点を有効にした後は、その重みを代入したり、長方形内の重みをまとめて変換したりできます。

`Point{x,y}` は登録する点、`Rectangle{left,bottom,right,top}` は半開長方形 `[left,right) × [bottom,top)` です。IDは登録順の0始まり添字です。同じ座標もIDごとに扱います。座標・重み・係数は符号付き64ビット整数です。入力検査が有効な場合、左右または上下が逆の長方形には `std::invalid_argument` を送出します。法 `Mod` は正の符号なし32ビット整数で、既定値は998244353です。

## `PointRectangleAffineKDTree`

```text
template <std::uint32_t Mod = 998244353>
class PointRectangleAffineKDTree;

explicit PointRectangleAffineKDTree(std::span<const Point> points = {});
```

後から有効にする点を含め、先に $M$ 個の座標を登録します。全点が非活性で始まります。$M$ は `INT_MAX` 以下です。 平均 $O(1+M\log(M+1))$ 時間・$O(M+1)$ 領域です。

## `set`

```text
void set(int id, std::int64_t value);
```

登録済みIDの点を有効にし、重みを代入します。有効な点に再代入できます。非活性の間の更新は引き継ぎません。 $O(1+\log(M+1))$ です。

## `range_affine`

```text
void range_affine(Rectangle rectangle, std::int64_t a, std::int64_t b);
```

長方形内の有効な点の重み `w` を `a*w+b` に変換します。空の長方形では変化しません。 $O(1+\sqrt M)$ です。

## `range_sum`

```text
std::uint32_t range_sum(Rectangle rectangle);
```

長方形内の有効点の重み和の非負剰余を返します。空の長方形の和は0です。 $O(1+\sqrt M)$ です。

## `all_sum`

```text
std::uint32_t all_sum() const noexcept;
```

全有効点の重み和の非負剰余を返します。 $O(1)$ です。

## `size`

```text
int size() const noexcept;
```

登録した点の数を返します。 $O(1)$ です。

## `active_count`

```text
int active_count() const noexcept;
```

有効な点の数を返します。 $O(1)$ です。

## `is_active`

```text
bool is_active(int id) const;
```

登録済みIDの点が有効かを返します。 $O(1)$ です。

## 使用例

```cpp
using Tree = sirkit::PointRectangleAffineKDTree<>;
const std::vector<Tree::Point> points{{1, 2}, {1, 2}};
Tree tree(points);
tree.set(0, 4);
tree.range_affine({0, 0, 3, 3}, 2, 3);
tree.set(1, 5);
const auto total = tree.range_sum({0, 0, 3, 3}); // 11 + 5 = 16
```

[使用例](../Examples/point_rectangle_affine_kd_tree_example.cpp) ／ [Verify](../Verify/current/submissions/dynamic_point_set_rectangle_affine_rectangle_sum.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/dynamic_point_set_rectangle_affine_rectangle_sum)
