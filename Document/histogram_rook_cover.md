# ヒストグラム盤のルーク被覆

`count_histogram_rook_covers(heights)` は、全マスを支配するルークの置き方の数を法 $998244353$ で返します。各列の下から `heights[i]` 個が盤のマスです。

同じ列か、同じ高さで切れ目のない行区間にルークがあれば、そのマスは支配されています。ルークが互いに隣り合う配置も数えます。

高さは `uint64_t` で、0の列も指定できます。マスがない盤に対する答えは1です。たとえば高さが `{2,1,2}` のときは17通りです。

列数 $n$、最大高さ $H$ に対して時間 $O(1+n^2\log(H+2))$、領域 $O(1+n)$ です。マスを一つずつ作らず、同じ幅の行区間をまとめて処理します。

[使用例](../Examples/histogram_rook_cover_example.cpp) ／ [Verify](../Verify/current/submissions/histogram_rook_cover_agc041_f.cpp)
