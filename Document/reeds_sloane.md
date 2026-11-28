# 合成数を法とする線形漸化式の復元

- `reeds_sloane(s, mod)` は、有限列 `s` に合う最短の定数係数線形漸化式の係数を返します。
- 戻り値を `c`、その長さを $L$ とすると、$i\ge L$ の項について $s_i=\sum_{j=0}^{L-1}c_j s_{i-j-1}\pmod{\text{mod}}$ が成り立ちます。空列や剰余がすべて0の列では $L=0$ です。
- 法は $1$ から $2^{32}-1$ までです。入力と戻り値の型は `uint32_t` で、入力は剰余に直して渡す必要はありません。

たとえば `s = {0, 1, 1, 2, 3, 5, 0, 5}` と `mod = 8` なら、`{1, 1}` が最短の漸化式です。復元は入力された範囲だけを保証します。

入力長 $N$、法の素因数の指数の総和 $E$ に対し、時間は $O(\sqrt{\text{mod}}+NE(N+\log(\text{mod}+1)))$、領域は $O(N+NE)$ です。

[使用例](../Examples/reeds_sloane_example.cpp) ／ [Verify](../Verify/current/submissions/reeds_sloane_abc129_f.cpp)
