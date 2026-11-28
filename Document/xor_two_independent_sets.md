# 二組の XOR 独立集合

整数を GF(2) のベクトルとして扱います。入力をできるだけ多く選び、各組がそれぞれ線形独立になるように二組へ分けます。等しい値でも入力番号が異なれば別要素です。

| API | 動作 |
|---|---|
| `XorTwoIndependentSets()` | 空から始める。 |
| `insert(value)` | `uint64_t` を追加し、最大個数が増えたとき `true` を返す。 |
| `rank()` | 二組の合計の最大個数を $O(1)$ で返す。 |
| `groups()` | 最大個数を実現する二組の0始まり入力番号を昇順で返す。$O(W)$。 |

$W=64$ として、$n$ 回の追加は合計 $O(nW+W^3)$ 時間、$O(W)$ 領域です。0はどちらの組にも入りません。例えば `1, 1, 2, 3, 3` からは合計4個を選べます。

[使用例](../Examples/xor_two_independent_sets_example.cpp) ／ [Verify](../Verify/current/submissions/xor_two_independent_sets_game_x.cpp)
