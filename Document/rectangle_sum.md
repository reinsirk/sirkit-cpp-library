# 重み付き点の長方形和

平面上の点が固定されていて、複数の長方形に含まれる重みの合計をまとめて求めたいときに使います。点の座標とクエリの境界は、符号付き64ビット整数です。

```cpp
const std::vector<sirkit::RectangleSumPoint> points{{0, 1, 5}, {0, 1, -2}, {2, 1, 10}};
const std::vector<sirkit::RectangleSumQuery> queries{{0, 0, 2, 2}, {0, 0, 3, 2}};
const auto sums = sirkit::rectangle_sum(points, queries); // {3, 13}
```

## `rectangle_sum`

```text
std::vector<std::int64_t> rectangle_sum(std::span<const RectangleSumPoint> points,
                                       std::span<const RectangleSumQuery> queries);
```

各点を `{x, y, weight}`、各長方形を `{left, bottom, right, top}` で渡します。範囲は `left <= x < right`、`bottom <= y < top` です。和をクエリと同じ順に返します。同じ座標の点も個別に加算でき、負の重みも使えます。

幅・高さのどちらかが0なら、和は0です。左右または上下を逆に指定すると、検査有効時は `std::invalid_argument` を送出します。

重みの和は `2^64` を法として求め、符号付き64ビット整数に対応させて返します。数学上の和が `std::int64_t` に収まる場合はそのまま返します。

点の個数を `N`、クエリ数を `Q` として、時間は `O((N+Q) log(N+Q+1))`、領域は `O(N+Q)` です。

[使用例](../Examples/rectangle_sum_example.cpp) ／ [Verify](../Verify/current/submissions/rectangle_sum.cpp) ／ [Luzhiled の関連実装](https://ei1333.github.io/library/other/static-point-add-rectangle-sum.hpp)
