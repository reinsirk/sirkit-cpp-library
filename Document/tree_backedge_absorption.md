# 祖先に戻る木上のランダムウォーク

`TreeBackedgeAbsorption(parents, backward_edges)` は、親から子へ向かう木に、葉以外から真の祖先へ戻る有向辺を加えます。根を `parents[v] = -1` で指定します。頂点は 0 始まりです。

根から出発し、毎回その頂点の出辺を一様に選んで、葉で停止します。重複辺は別々の選択肢です。

- `expected_steps<Field>()` は停止までの移動回数の期待値です。
- `expected_cost<Field>(costs)` は各頂点を出発するたびに `costs[v]` を払ったときの合計の期待値です。葉のコストは加算しません。

`Field` は有限体などの数値型です。消去に必要な係数が 0 になると `std::nullopt` を返します。素数を法にした計算では別の素数で試せます。

頂点が `n` 個、追加した辺が `m` 本なら、構築時間と使用領域は `O(n+m)` です。期待値計算には `O((n+m) log²(n+m+1))` 回の体演算が必要で、除算はそのうち `O(n)` 回に収まります。

[使用例](../Examples/tree_backedge_absorption_example.cpp) · [Verify問題](https://atcoder.jp/contests/jsc2024-final/tasks/jsc2024_final_e)
