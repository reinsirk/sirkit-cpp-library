# 2冪長の整列区間への分割

`dyadic_interval_cover(left,right)` は半開区間 `[left,right)` を、長さが $2^k$ で始点も $2^k$ の倍数である区間に分けます。区間数が最少となる分割を、左から順に `pair{left,right}` の列で返します。

たとえば `[3,9)` は `[3,4)`、`[4,8)`、`[8,9)` に分かれます。負の座標も指定できます。両端が等しければ空列です。

分割後の区間数を $k$ とすると、時間・領域はともに $O(1+k)$ です。

[使用例](../Examples/dyadic_interval_cover_example.cpp) ／ [Verify](../Verify/current/submissions/dyadic_interval_cover_abc349_d.cpp)
