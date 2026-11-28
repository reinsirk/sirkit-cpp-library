# 隣接交換の最小回数を偶数・奇数別に取得

`adjacent_swap_distances_by_parity(from, to, compare={})` は、隣接する2要素の交換で `from` を `to` に一致させる最小回数を偶数・奇数別に求めます。

- 返り値の `even` と `odd` は `optional<uint64_t>`。その偶奇で一致させられない項目は `nullopt` になります。同じ値を交換しても1回です。
- 比較と入力列の要件は [adjacent_swap_distance](adjacent_swap_distance.md) と同じです。長さは `INT_MAX` 以下。
- 長さ $n$ の列で $O((n+1)\log(n+2))$ 時間、$O(n+1)$ 領域です。

たとえば `{1,2,3,1}` を `{1,3,2,1}` に変えるには、奇数回なら1回、偶数回なら6回が最小です。

[使用例](../Examples/adjacent_swap_parity_example.cpp) ／ [Verify](../Verify/current/submissions/adjacent_swap_parity_arc214_e.cpp)
