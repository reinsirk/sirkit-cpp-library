# Alien Trick

「ちょうど $k$ 個選んだときの最小費用」を、1個ごとのペナルティを加える問題に置き換えます。個数ごとの最小費用が整数の離散凸列で、実行可能な個数が連続する場合に使えます。

| API | 動作 |
|---|---|
| `CardinalityOptimum<Cost>{cost, least, most}` | ペナルティ込みの最小値と、それを達成する個数の最小・最大。 |
| `choose_cardinality_optimum(a, b)` | 費用が小さい方を選ぶ。同額なら個数の両端を統合する。 |
| `alien_minimum_cost(k, lower, upper, evaluate)` | `evaluate(p)` が「費用 $+p\times$ 個数」を最小化し、`CardinalityOptimum` を返す。ペナルティを閉区間 `[lower, upper]` から探し、元の費用を返す。 |

区間に $k$ を最適にする整数ペナルティがなければ `std::nullopt` です。評価回数は区間内の整数が $R$ 個なら $O(\log(R+1))$ 回です。復元に使うペナルティ、個数と途中の計算がすべて `Cost` に収まることが前提です。

[使用例](../Examples/alien_trick_example.cpp) ／ [Verify](../Verify/current/submissions/alien_trick_abc462_f.cpp)
