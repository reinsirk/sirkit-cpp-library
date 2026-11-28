# 単純無向グラフの次数列

- `is_graphical_degree_sequence(degrees)` は、指定した次数を持つ単純無向グラフが存在するかを返します。
- `realize_degree_sequence(degrees)` は、そのようなグラフを0始まりの頂点番号の辺リストで返します。作れない場合は `nullopt` です。
- 頂点数 $n$、辺数 $m$ に対し、判定は $O(n+1)$、構築は $O(n+m+1)$ 時間です。構築時は辺リストを除いて $O(n+1)$ 領域を使います。

入力の順番を頂点番号として使います。たとえば `[1,1,2]` なら、頂点2と頂点0、頂点2と頂点1をつなげます。空の入力は頂点と辺がないグラフです。

[使用例](../Examples/degree_sequence_example.cpp) ／ [Verify（構築）](../Verify/current/submissions/degree_sequence_yuki1519.cpp) ／ [Verify（判定）](../Verify/current/submissions/degree_sequence_yahoo2018_e.cpp)
