# 行区間が指定の階数に達する右端

`interval_matrix_rank_ends<Mod>(rows, rank)` は、各左端 `l` について行区間 `[l,r)` の階数が `rank` 以上になる最小の `r` を返します。そのような区間がなければ `nullopt` です。

- `rows` は長方形の `uint32_t` 行列です。各値は素数 `Mod` で割った余りとして扱います。既定の法は $998244353$ です。
- `rank=0` の最小の右端は `l` です。空の行列には空の結果を返します。
- 行数 $H$、列数 $W$ として $O(HW(W+\log \mathrm{Mod})+H)$ 時間、$O(W^2+H)$ 領域です。

たとえば行が順に `[1,0]`、`[2,0]`、`[0,1]` のとき、階数2に達する右端は `[3,3,nullopt]` です。

階数の計算は決定的です。Verify では色の制約を行列に変換する段階で乱数を使います。

[使用例](../Examples/interval_matrix_rank_example.cpp) ／ [Verify](../Verify/current/submissions/interval_matrix_rank_abc399_g.cpp)
