# 長方形の和集合の面積

軸に平行な長方形を重ねたときの面積を求めます。重複部分は1回だけ数えます。

```cpp
std::vector<sirkit::AreaRectangle> rectangles{{0, 0, 2, 2}, {1, 1, 3, 3}};
auto area = sirkit::rectangle_union_area(rectangles);
```

## `rectangle_union_area`

```text
std::uint64_t rectangle_union_area(std::span<const AreaRectangle> rectangles);
```

長方形は `{left, bottom, right, top}` の順に64ビット符号付き整数で指定し、`left <= right` と `bottom <= top` を満たす必要があります。通常版で座標の順序が逆なら `std::invalid_argument` です。有効な長方形のうち幅か高さが0のものは無視します。

面積が `std::uint64_t` の範囲を超えると `std::overflow_error` を送出します。長方形 $n$ 枚に対して $O(n\log(n+1))$ 時間・$O(n)$ 領域です。

[使用例](../Examples/rectangle_union_area_example.cpp) ／ [Verify](../Verify/current/submissions/rectangle_union_area.cpp)
