# 数直線上のチップ発火

同じ整数位置 $x$ にチップが2個あれば、1個を $x-1$、もう1個を $x+1$ に移します。

`line_chip_firing(positions)` は、重複がなくなるまで移動を繰り返した結果を返します。`positions` は最終位置の昇順列、`operations` は移動操作の総回数です。どちらも操作する順序によらず決まります。

たとえば `{0,0,0}` を渡すと、最終位置は `{-1,0,1}`、操作回数は1です。

位置は `int64_t` で、チップの個数は $2^{20}$ まで指定できます。最終位置が型の範囲を超えると `overflow_error` が発生します。チップ数 $n$ に対して時間 $O(1+n\log(n+1))$、領域 $O(1+n)$ です。

[使用例](../Examples/line_chip_firing_example.cpp) ／ [Verify](../Verify/current/submissions/line_chip_firing_yuki3801.cpp)
