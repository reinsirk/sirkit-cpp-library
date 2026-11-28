# 線形マトロイドパリティ

ベクトルの対をいくつか選び、選んだすべてのベクトルが線形独立になるときの、対の最大数を求めます。

| API | 動作 |
|---|---|
| `LinearMatroidParity<Field>(dimension)` | 有限体上の $D$ 次元ベクトルを扱う。初期化は $O(D^2)$ 時間・領域。 |
| `add_pair(left, right, coefficient)` | 疎なベクトルの対を追加する。各成分は `{index, value}`、0始まり。範囲外は `std::out_of_range`。同じ添字の成分は加算する。要素数を $a,b$ として $O(a+b+ab)$ 時間。 |
| `sample_size()` | 最大数の下界を返す。$O(D^3)$ 時間、$O(D^2)$ 追加領域。 |

位数 $q$ の体から、各対の `coefficient` を独立一様に選びます。真の最大数を $r$ とすると、結果が $r$ より小さくなる確率は高々 $r/q$ です。大きい素数法を使い、必要に応じて独立な乱数でやり直して最大値を採ります。

[使用例](../Examples/linear_matroid_parity_example.cpp) ／ [Verify](../Verify/current/submissions/linear_matroid_parity_abc482_g.cpp)
