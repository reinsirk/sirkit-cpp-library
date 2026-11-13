# オイラー路

`Graph/Eulerian_Trail.cpp`の`EulerianTrail<true>`は有向、`EulerianTrail<false>`は無向グラフを扱う。別名は`DirectedEulerianTrail`・`UndirectedEulerianTrail`。自己辺と平行辺を許し、各辺は`add_edge`の返したIDで区別する。

`find()`は全辺を一度ずつ通る路の`optional<Trail>`を返す。存在しなければ`nullopt`。`Trail.vertices[i]`から`vertices[i+1]`へ、`Trail.edges[i]`の辺を通る。無向辺で追加時と逆向きに通ったかどうかも頂点列から分かる。

`find(start)`は開始頂点を指定し、`find(start,true)`は閉路だけを求める。省略値-1では次数条件から始点を選ぶ。0辺なら1頂点だけの路、0頂点0辺なら空の頂点列・辺列を持つ路とする。辺を持つ成分が複数あれば1本で全辺を通れないため`nullopt`で、孤立頂点を路へ加えることはしない。

次数条件を確かめてCSRを作り、辺を消費しながら頂点をスタックへ積むHierholzer法を使う。行き止まりから逆順に取り出した列を最後に反転する。無向版は同じIDが両端に現れるので、使用済みの辺を記録する。有向版は各IDが1回だけ現れ、その配列を持たない。O(N+M)時間・領域。入力の辺配列を変更しないため、開始点や閉路条件を変えて再照会できる。

端点・辺IDの入力検査は`SKIP_RUNTIME_CHECKS`で除くが、存在条件と全辺を使い切ったかの判定は維持する。コピーは独立所有、ムーブ元は0頂点0辺になる。結果配列も入力とは独立して所有する。

[参考](https://ei1333.github.io/library/graph/others/eulerian-trail.hpp)は存在条件の確認に使い、破壊的な隣接リスト実装は転記していない。[使用例](../Examples/eulerian_trail_example.cpp) · [独立参照](../Verify/current/tests/new_eulerian_reference_test.cpp)
