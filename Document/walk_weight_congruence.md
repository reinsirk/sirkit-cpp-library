# 歩道の重みの合同類

有向グラフで、辺重みの総和が指定した余りになる歩道を調べます。歩道では同じ頂点や辺を何度でも通れます。

`WalkWeightCongruence(n, edges)` に `WalkCongruenceEdge{from, to, weight}` を渡します。頂点は 0 始まりで、重みは符号付きの `int32_t` です。

- `period(v)` は `v` へ戻る歩道の重みの最大公約数です。重みが 0 の歩道しかなければ 0 を返します。`periods()` では強連結成分ごとの周期を一度に取得できます。
- `relation(u, v)` は同じ強連結成分で `WalkWeightClass{offset, period}` を返します。別成分なら `nullopt` です。
- 結果の `matches(residue, modulus)` は、総重みが `residue` と法 `modulus` で合同な歩道の有無を返します。法は正の整数です。

周期が正なら、重みはすべて `offset` と法 `period` で合同になります。整数として `offset` そのものを実現できるとは限りません。周期が 0 の場合、同成分内の歩道には重み `offset` だけが現れます。

構築は `O(n + m log(nW + 2))` 時間と `O(n + m)` 領域です。`W` には辺重みの絶対値の上限を使います。周期と合同類の取得は `O(1)`、余りの判定は `O(log(modulus + 1))` 時間です。

[使用例](../Examples/walk_weight_congruence_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc306/tasks/abc306_g)
