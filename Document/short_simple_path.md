# 頂点数を指定した単純パス

- `find_short_simple_path(g, k, starts, ends)` は、ちょうど $k$ 頂点を通る単純パスを頂点列で返します。見つからなければ `nullopt` です。
- `g[u]` は頂点 $u$ からの行き先です。`starts` と `ends` には始点・終点の候補を0始まりで渡します。$1\le k\le8$、頂点数は $998244353$ 未満です。

たとえば頂点 $u$ を通る長さ5の閉路は、$u$ をグラフから除き、$k=4$ のパスとして探せます。元のグラフで $u$ から辺が出る先を始点候補、$u$ に辺が入る元を終点候補とします。

$n$ 頂点、$m$ 辺、$D=\binom{k}{\lfloor k/2\rfloor}$ とすると、時間は $O(k(n+m)D^2(D+k))$、領域は $O(m+nD(D+k))$ です。始点と終点の候補を読む時間を別途加えます。乱数は使いません。

[使用例](../Examples/short_simple_path_example.cpp) ／ [Verify](../Verify/current/submissions/short_simple_path_yuki408.cpp)
