# 重み付き Slope Trick

整数座標に折点がある凸区分線形関数を管理します。`WeightedSlopeTrick` の初期値は `f(x) = 0` です。

| API | 内容 |
| --- | --- |
| `add_abs(a, w)` | $w\lvert x-a\rvert$ を加える。 |
| `add_left_hinge(a, w)` / `add_right_hinge(a, w)` | $w\max(a-x,0)$ / $w\max(x-a,0)$ を加える。 |
| `min_value()` / `argmin()` | 最小値 / 最小点の閉区間。限りのない端は `std::nullopt`。$O(1)$。 |
| `shift(a)` / `shift(a, b)` | $f(x-a)$ / $\min_{x-b\le y\le x-a} f(y)$ に変える。$O(1)$。 |
| `clear_left()` / `clear_right()` | $\min_{x\le y} f(y)$ / $\min_{y\le x} f(y)$ に変える。消す折点の数に比例。 |

```cpp
sirkit::WeightedSlopeTrick cost;
cost.add_abs(2, 3);
cost.add_abs(5, 1);
auto minimum = cost.min_value(); // 3
```

折点が $K$ 個あり、追加時に $C$ 個を移す場合は償却 $O((C+1)\log(K+2))$ 時間かかります。`add_constant(c)` は $O(1)$、`eval(x)` は $O(K+1)$ です。重みには `std::uint64_t`、座標と値には `std::int64_t` を使います。

[使用例](../Examples/weighted_slope_trick_example.cpp) ／ [Verify](../Verify/current/submissions/weighted_slope_trick_selling_cars.cpp) ／ [Nyaanの解説](https://nyaannyaan.github.io/library/data-structure/slope-trick-weighted.hpp)
