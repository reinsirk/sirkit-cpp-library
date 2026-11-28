# 等差にずらした列の最小剰余の総和

`sum_minimum_modular_shifts(count, modulus, step, offsets, start=0)` は、各 $k$ に対して `(start + step * k + offset) mod modulus` の最小値を取り、それを $0 \le k < \text{count}$ で合計します。各 $k$ で `offsets` 全体を調べる代わりに、剰余の順序が変わる位置を求めます。

- `count` と `modulus` は `uint32_t`。差 `step`、初期値 `start`、列 `offsets` の値は負の整数も指定できます。
- 答えは `uint64_t`。空の `offsets` や法0には `std::invalid_argument` を投げます。有効な入力で `count=0` なら0です。
- 列の長さを $n$、異なる剰余の個数を $u$、法を $m$ とすると、$O(n\log(n+1)+u\log(m+1))$ 時間、$O(n)$ 領域です。1回分、または `step` が法の倍数なら $O(n)$ 時間です。

たとえば `offsets={1,3}`、法5、初期値0、公差3で3回分を求めると、最小値は順に $1,1,2$ で総和は4です。

[使用例](../Examples/modular_shift_minimum_example.cpp) ／ [Verify](../Verify/current/submissions/modular_shift_minimum_arc182_e.cpp)
