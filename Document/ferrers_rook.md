# Ferrers盤のルーク数

各列の先頭 `heights[i]` 個のマスにルークを置けます。列の順序は任意です。

- `ferrers_rook_counts(heights)[k]` は、同じ行や列を使わずに $k$ 個置く方法の数です。長方形の盤にも使えます。
- `ferrers_hit_counts(heights)[k]` は、$n\times n$ 盤に行・列ごとに1個ずつ置き、うち $k$ 個が許可マスにある配置の数です。この関数では各 `heights[i]` は $n$ 以下です。

たとえば `heights = {1,2}` なら、置けるマスは3つなのでルーク数は `{1,3,1}`、順列に対応する配置の数は `{0,1,1}` です。

列数 $n<2^{20}$。法は $998244353$。どちらも時間 $O(1+n\log^2(n+1))$、領域 $O(1+n\log(n+1))$ です。

[使用例](../Examples/ferrers_rook_example.cpp) ／ [Verify](../Verify/current/submissions/ferrers_rook_abc272_ex.cpp)
