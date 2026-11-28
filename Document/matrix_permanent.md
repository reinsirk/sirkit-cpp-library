# 行列のパーマネント

行と列を1対1で対応させ、その組で選んだ要素の積をすべて足します。隣接行列が0と1なら、二部グラフの完全マッチングの個数になります。

| API | 動作 |
|---|---|
| `matrix_permanent<T>(n, value_at, one = T{1})` | `value_at(row,col)` が要素の $n\times n$ 行列のパーマネントを返す。空行列なら `one`。 |

`T{}` が0、`one` が1です。足し算・引き算・掛け算を使います。合成数を法とする剰余でも使えます。符号付き組み込み整数型では、途中の値も型に収まるようにしてください。法 $2^{64}$ の計算には `std::uint64_t` を使えます。

環演算が定数時間なら $O(n2^n)$ 時間、$O(n^2)$ 領域です。$n\ge64$ は `std::length_error` になります。

[使用例](../Examples/matrix_permanent_example.cpp) ／ [Verify](../Verify/current/submissions/matrix_permanent_dp_o.cpp)
