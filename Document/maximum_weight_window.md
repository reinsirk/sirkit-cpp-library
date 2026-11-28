# 位置を選べる長方形の最大重み

`maximum_weight_window(points, width, height)` は、決まった幅・高さの長方形を動かして、内側の点の重みの和を最大化します。点は `WeightedWindowPoint{x, y, weight}` です。同じ座標や負の重みも扱います。

```cpp
auto best = sirkit::maximum_weight_window(points, 5, 5);
// best.weight が重みの和。best.left と best.bottom が長方形の左下。
```

整数の左下を選び、長方形 `[left, left + width) × [bottom, bottom + height)` を調べます。整数格子上で上端と右端も含めるには、幅と高さをそれぞれ1増やします。左下を実数で選べる問題では、座標と辺の長さを2倍し、幅と高さに1を足すと半整数の位置も調べられます。点を含まない長方形を選べるため、答えは常に0以上です。

点数を $n$ とすると $O(n\log(n+1))$ 時間、$O(n+1)$ 領域です。重みの絶対値の合計は `INT64_MAX` 以下にします。

[使用例](../Examples/maximum_weight_window_example.cpp) ／ [重み付きVerify](../Verify/current/submissions/maximum_weight_window.cpp) ／ [個数のVerify](../Verify/current/submissions/maximum_weight_window_apples.cpp)
