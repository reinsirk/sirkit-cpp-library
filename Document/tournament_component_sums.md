# トーナメント全体の強連結成分数の和

`tournament_component_sums<T>(n)` は、各頂点ペアに片方向の辺が 1 本ある有向グラフを対象にします。小さい番号から大きい番号への辺がちょうど `m` 本あるものを集め、強連結成分数を足した値が戻り値の `answer[m]` です。

3 頂点でそのような向きが 1 本なら、該当する 3 グラフの成分数は 3、1、3 で、戻り値の添字 1 は 7 です。法付きの型 `T` を渡すとその法で数えられます。

強連結成分を順に並べ、途中の境界の前へ外側から入る辺がないことを使って数えます。全 `m` を求める計算量は `O((n+1)^5)` 時間・`O((n+1)^3)` 領域です。頂点数が小さい場合向けです。

[使用例](../Examples/tournament_component_sums_example.cpp) · [Verify問題](https://atcoder.jp/contests/arc163/tasks/arc163_d) · [トーナメントの解説](https://info.atcoder.jp/entry/algorithm_lectures/tournament)
