# 多項式のシフトの重み付き和

`weighted_taylor_shifts(f, shifts, weights)` は、$\sum_i \mathrm{weights}_i f(x+\mathrm{shifts}_i)$ を返します。たとえば $f(x)=x^2$、シフト `{-1,1}`、重み `{1,1}` なら結果は $2x^2+2$ です。

法は $998244353$ です。シフトと重みは同じ長さの整数ベクトルで、負の値や重複するシフトも指定できます。結果の係数数は `f` と同じです。

シフト数 $m$、係数数 $d\le 2^{22}$ に対して、時間は $O((m+d+1)\log^2(m+d+2))$、領域は $O(1+m+d)$ です。

[使用例](../Examples/weighted_taylor_shifts_example.cpp) ／ [Verify](../Verify/current/submissions/weighted_taylor_shifts_yuki3793.cpp)
