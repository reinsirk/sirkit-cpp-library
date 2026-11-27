# オフラインの点加算・長方形和

点の追加と長方形の照会を、与えた順番で処理します。照会ではその時点までに追加済みの点が対象です。

座標と重みは符号付き64ビット整数で、負の値も使えます。入力検査が有効な場合、左右または上下が逆の長方形には `std::invalid_argument` を送出します。

## `PointAddRectangleSumOperation`

```text
struct PointAdd { std::int64_t x, y, weight; };
struct RectangleSumRequest { std::int64_t left, bottom, right, top; };
using PointAddRectangleSumOperation = std::variant<PointAdd, RectangleSumRequest>;
```

`PointAdd` は点を1つ追加します。同じ座標への追加も個別に数えます。`RectangleSumRequest` は半開長方形 `[left,right) × [bottom,top)` を照会します。

## `offline_point_add_rectangle_sum`

```text
inline std::vector<std::int64_t> offline_point_add_rectangle_sum(
    std::span<const PointAdd> initial, std::span<const PointAddRectangleSumOperation> operations);
```

初期点と全操作を受け取り、照会の答えを順に返します。空の長方形の和は0です。和は $2^{64}$ を法とし、符号付き64ビット整数に対応付けて返します。実際の和がその範囲内なら厳密な和を返します。途中の集計値はその範囲を超えても構いません。

照会がある場合、初期点数と追加数の合計に照会数の2倍を足した値は $2^{32}-1$ 未満です。 初期点数 $n$、操作数 $q$ に対して $O((n+q)\log^2(n+q+2))$ 時間・$O(n+q)$ 領域です。

## 使用例

```cpp
const std::vector<sirkit::PointAdd> initial{{0, 0, 4}};
const std::vector<sirkit::PointAddRectangleSumOperation> operations{
    sirkit::RectangleSumRequest{0, 0, 2, 2},
    sirkit::PointAdd{0, 0, -1},
    sirkit::RectangleSumRequest{0, 0, 2, 2},
};
const auto answers = sirkit::offline_point_add_rectangle_sum(initial, operations);
// answers は {4, 3}
```

[静的な長方形和](rectangle_sum.md) は全照会で点集合を共有します。[動的2次元 Fenwick Tree](dynamic_fenwick_2d.md) は操作が届くたびに処理できます。

[使用例](../Examples/offline_point_add_rectangle_sum_example.cpp) ／ [Verify](../Verify/current/submissions/offline_point_add_rectangle_sum.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/point_add_rectangle_sum)
