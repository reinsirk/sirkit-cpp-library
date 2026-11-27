# 弦グラフ判定

無向グラフに長さ4以上の誘導閉路があるかを判定し、結果の証明を返します。

```cpp
const auto result = sirkit::recognize_chordal_graph(n, edges);
if (result.is_chordal)
{
    const auto &order = result.perfect_elimination_order;
}
else
{
    const auto &cycle = result.induced_cycle;
}
```

## `recognize_chordal_graph`

```text
ChordalGraphResult recognize_chordal_graph(int n, std::span<const std::pair<int, int>> edges);
```

`is_chordal` が真なら `perfect_elimination_order` に完全除去順序が入ります。この順に頂点を取り除くと、取り除く頂点の未除去の隣接頂点が毎回クリークをなします。

それ以外の場合は `induced_cycle` に長さ4以上の閉路の頂点が巡回順で入ります。閉路の頂点同士を結ぶ辺は列中で隣り合う2頂点の辺と、先頭・末尾を結ぶ辺だけです。

`edges` は0始まりの無向辺 `{u, v}` の列です。自己辺は無視し、多重辺は1本として扱います。頂点数を $n$、入力辺数を $m$ とすると $O(n+m)$ 時間・領域です。

[使用例](../Examples/chordal_graph_example.cpp) ／ [Verify](../Verify/current/submissions/chordal_graph_recognition.cpp) ／ [最大基数探索の原論文](https://doi.org/10.1137/0213035)
