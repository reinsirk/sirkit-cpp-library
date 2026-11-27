# 長方形加算と点取得

複数の長方形に重みを加え、点を含む長方形の重みの和を求めます。照会にはその時点までに追加した長方形が反映されます。

座標と重みは符号付き64ビット整数で、負の値も使えます。入力検査が有効な場合、左右または上下が逆の長方形には `std::invalid_argument` を送出します。

## `RectangleAddPointGetOperation`

```text
struct RectangleAdd { std::int64_t left, bottom, right, top, weight; };
struct RectanglePointGet { std::int64_t x, y; };
using RectangleAddPointGetOperation = std::variant<RectangleAdd, RectanglePointGet>;
```

`RectangleAdd` は半開長方形 `[left,right) × [bottom,top)` に重みを加えます。幅または高さが0なら変化しません。`RectanglePointGet` は点 `(x,y)` の値を照会します。

## `offline_rectangle_add_point_get`

```text
inline std::vector<std::int64_t> offline_rectangle_add_point_get(
    std::span<const RectangleAdd> initial, std::span<const RectangleAddPointGetOperation> operations);
```

初期長方形と全操作を受け取り、照会の答えを順に返します。同じ長方形を複数回追加した場合、それぞれの重みを数えます。和は $2^{64}$ を法とし、符号付き64ビット整数に対応付けて返します。

照会がある場合、初期長方形と追加の合計を2倍して照会数を足した値が $2^{32}-1$ を超えると `std::length_error` を送出します。 初期長方形の数 $n$、操作数 $q$ に対して $O((n+q)\log^2(n+q+2))$ 時間・$O(n+q)$ 領域です。

## 使用例

```cpp
const std::vector<sirkit::RectangleAdd> initial{{0, 0, 3, 3, 5}};
const std::vector<sirkit::RectangleAddPointGetOperation> operations{
    sirkit::RectanglePointGet{1, 1},
    sirkit::RectangleAdd{1, 1, 2, 2, -3},
    sirkit::RectanglePointGet{1, 1},
    sirkit::RectanglePointGet{2, 1},
};
const auto answers = sirkit::offline_rectangle_add_point_get(initial, operations);
// answers は {5, 2, 5}
```

[使用例](../Examples/offline_rectangle_add_point_get_example.cpp) ／ [Verify](../Verify/current/submissions/rectangle_add_point_get.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/rectangle_add_point_get)
