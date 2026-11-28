# トーナメントの有向パス

トーナメントは、相異なる頂点の組ごとにどちらか一方向に辺があるグラフです。`tournament_path(n, edge)` は、頂点 `0..n-1` を一度ずつ通る有向パスを返します。`edge(u, v)` が `true` なら、`u → v` の辺があるという意味です。

```cpp
auto path = sirkit::tournament_path(3, [](int u, int v) {
    return (u + 1) % 3 == v;
});
```

両側のパスを先頭同士の辺の向きで併合するため、勝敗に循環があっても使えます。問い合わせは $O(n\log(n+1))$ 回、追加領域は $O(n)$ です。`n=500` での問い合わせは最大3989回です。

[使用例](../Examples/tournament_path_example.cpp) ／ [Verify](../Verify/current/submissions/tournament_path_yuki2085.cpp) ／ [AtCoder Algorithm Lectures](https://info.atcoder.jp/entry/algorithm_lectures/tournament)
