# 異なる法で取った剰余の総和を一括計算

`modular_shift_sums(limit, terms)` は、$x=0,\dots,\text{limit}$ のそれぞれに対し、全項の `(x + offset) mod modulus` の和を返します。各項に別々の法を指定できます。

- 各項は `ModularShiftTerm{offset, modulus}`。負の `int64_t` の `offset` も使え、剰余は非負です。法は正の `uint32_t` で、0なら `std::invalid_argument` になります。
- 返り値は長さ `limit + 1` の `vector<uint64_t>`。同じ項も入力回数分数えます。和が `uint64_t` の上限を超える場合は $2^{64}$ で折り返します。
- 項数を $n$、求める最大の $x$ を $L$ とすると、$O(n\log(n+1)+(L+1)\sqrt{n+1})$ 時間、$O(n+L+1)$ 領域です。

たとえば `terms={{1,3},{2,5}}` で `limit=3` なら、結果は `{3,5,4,1}` です。

[使用例](../Examples/modular_shift_sums_example.cpp) ／ [Verify](../Verify/current/submissions/modular_shift_sums_abc483_f.cpp)
