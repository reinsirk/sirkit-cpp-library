# ヨセフスの問題

円環に $0,1,\ldots,n-1$ を順に並べ、番号 $0$ を1番目として数えます。`step` 番目を除いたら、その直後を1番目として数え直します。

- `josephus_survivor(n, step)` は最後に残る番号を返します。`n` と `step` は正の `uint64_t` です。時間は $O(\min(n,\mathrm{step}\log(n+1)))$ で、間隔が1か2なら $O(1)$ です。
- `josephus_order(n, step)` は除去順を返し、最後に残る番号を末尾に付けます。`step` は正で、`n` は非負の `int` です。時間は $O(1+n\log(n+1))$、領域は $O(1+n)$ です。

たとえば `n=7, step=3` の除去順は `{2,5,1,6,4,0,3}` で、最後に残る番号は `3` です。

[使用例](../Examples/josephus_example.cpp) ／ [Verify](../Verify/current/submissions/josephus_survivor_aoj0085.cpp)
