# 標準ヤング盤とドミノ盤の数え上げ

行の長さを上から広義単調減少に並べて `shape` に渡します。末尾の0は省略できます。マス数の合計は `uint32_t` の範囲内です。答えは法 `Mod` の剰余で、既定値は $998244353$ です。合成数の法も使えます。

- `standard_young_tableau_count<Mod>(shape)` は、各行・各列で番号が増加する標準ヤング盤の個数を返します。`[3,2]` なら5通りです。
- `domino_tableau_count<Mod>(shape)` は、毎回ヤング図形を保つように右端から横または縦のドミノを除き、空にする順序の個数を返します。`[3,3]` なら3通りです。
- 空図形は1通りです。行数 $H$、幅 $W$、マス数 $N$、法の異なる素因数数 $k$ として、$O(H+N(k+1)+(k+1)\log(\mathrm{Mod}+1))$ 時間、$O(H+W+k+1)$ 領域です。

[使用例](../Examples/young_tableau_count_example.cpp) ／ [Verify（標準）](../Verify/current/submissions/young_tableau_numbering_blocks.cpp) ／ [Verify（ドミノ）](../Verify/current/submissions/domino_tableau_yuki2149.cpp)
