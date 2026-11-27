# 最小包含円

整数座標の点列をすべて含む、半径が最小の円を求めます。円周上の点も判定できます。

```cpp
std::vector<sirkit::MinimumEnclosingCircle::Point> points{{0, 0}, {1, 0}, {2, 0}};
auto circle = sirkit::minimum_enclosing_circle(points);
assert(circle->on_boundary(points[0]));
assert(!circle->on_boundary(points[1]));
```

## `minimum_enclosing_circle`

```text
std::optional<MinimumEnclosingCircle> minimum_enclosing_circle(
    std::span<const MinimumEnclosingCircle::Point> points, std::uint64_t seed = 0);
```

`MinimumEnclosingCircle::Point` は `std::pair<std::int32_t, std::int32_t>` です。各座標の範囲は $[-10000,10000]$ で、同じ点も複数渡せます。空の点列の戻り値は `std::nullopt` です。通常版の構築と判定で範囲外の座標を渡すと `std::out_of_range` を送出します。

点の処理順を `seed` でシャッフルします。処理順を一様に選ぶと期待 $O(N)$ 時間、最悪 $O(N^3)$ 時間です。追加領域は $O(N)$ です。

## `center`

```text
std::pair<double, double> center() const noexcept;
```

円の中心を `{x, y}` の近似値で返します。$O(1)$ 時間です。

## `radius_squared`

```text
double radius_squared() const noexcept;
```

半径の二乗の近似値を返します。$O(1)$ 時間です。

## `contains`

```text
bool contains(Point point) const;
```

点が円内または円周上にあれば真を返します。構築時と同じ座標範囲で、整数演算で厳密に判定します。$O(1)$ 時間です。

## `on_boundary`

```text
bool on_boundary(Point point) const;
```

点が円周上にあれば真を返します。入力範囲と判定精度は `contains` と共通です。$O(1)$ 時間です。

[使用例](../Examples/minimum_enclosing_circle_example.cpp) ／ [Verify](../Verify/current/submissions/minimum_enclosing_circle.cpp)
