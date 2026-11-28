# 隣り合わない要素の選択

数列から隣り合わない要素を **ちょうど** $k$ 個選び、選んだ値の和を最大化します。値がすべて負の場合も、指定個数を選びます。

| API | 動作 |
|---|---|
| `NonadjacentSelection<Score>(weights, limit)` | $0\le k\le K$ の答えを作る。$K=\min(\text{limit},\lceil n/2\rceil)$。 |
| `sums()` | $k$ 番目がちょうど $k$ 個選んだ最大和。参照を $O(1)$ で返す。 |
| `positions(k)` | 最大和になる0始まりの添字を昇順で返す。$k>K$ なら `std::nullopt`。 |

`limit` の既定は上限なし、`Score` の既定は `std::int64_t` です。`Score{}` は0で、途中の加減算も `Score` に収まることが前提です。比較と加減算が定数時間なら、構築は $O(n+K\log(n+1))$ 時間、使用領域は $O(n+K)$。復元は $O(n+K)$ 時間・領域です。

`{4, 8, 6}` なら、1個の最大和は8、2個の最大和は10で、添字は `{0, 2}` です。

[使用例](../Examples/nonadjacent_selection_example.cpp) ／ [Verify](../Verify/current/submissions/nonadjacent_selection_abc218_h.cpp)
