# 区間を覆う部分集合の数え上げ

`count_interval_cover_subsets<T>(left, right, choices)` は、半開区間 `[left, right)` 全体を覆う選び方を集計します。同じ区間が複数あれば、それぞれ選択できます。

各候補は `IntervalCoverChoice<T>{left, right, omit, take}` です。選んだ区間には `take`、選ばなかった区間には `omit` を使い、全区間の係数を掛け合わせてから選び方ごとに足します。両方を省くと単純な個数になります。

空の対象区間ではすべての部分集合が条件を満たします。座標は `int64_t` です。個数が型に収まらない場合は、法を使う数値型を `T` に指定できます。

候補を左端順に処理し、対象の左端から途切れずに覆えた範囲だけを覚えます。候補の数を `m` とすると、計算量は `O(1+m log(m+1))` 時間・`O(m+1)` 領域です。

[使用例](../Examples/interval_cover_subsets_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc450/tasks/abc450_f)
