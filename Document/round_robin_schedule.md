# 総当たり戦の組み合わせ

- `RoundRobinSchedule(n)` は、すべての2人組が1回ずつ対戦する日程を作ります。1ラウンドで各選手が行う試合は1つまでです。
- `round_count()` は必要なラウンド数を返します。$n\le1$ なら0、偶数人なら $n-1$、奇数人なら $n$ です。
- `partner(r,u)` はラウンド $r$ の相手を返します。休みなら `-1` です。
- `round_of(u,v)` は2人の対戦ラウンドを返します。同じ選手を渡すと `-1` です。

`size()` は選手の人数を返します。
選手とラウンドはどちらも0始まりです。すべての操作は $O(1)$ 時間で、日程表を保存する領域は不要です。

[使用例](../Examples/round_robin_schedule_example.cpp) ／ [Verify](../Verify/current/submissions/round_robin_schedule_arc208_d.cpp)
