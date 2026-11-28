# マッチングの条件を満たすラベル付き木の個数

戻り値の `n` 番目に、頂点 `1..n` のラベル付き木の個数が法998244353で入ります。

- `labeled_tree_exposable_root_counts(limit)` は、頂点1を使わない最大マッチングが存在する木を数えます。未訪問の隣接頂点に進むゲームでは、頂点1から始めて手番を持つ側が負ける木の個数に当たります。
- `labeled_tree_perfect_matching_counts(limit)` は、全頂点が辺の端点となる完全マッチングを持つ木を数えます。

```cpp
auto losing = sirkit::labeled_tree_exposable_root_counts(4);
auto perfect = sirkit::labeled_tree_perfect_matching_counts(4);
// losing[4] == 3、perfect[4] == 12
```

前者は根付き木の指数型母関数をニュートン法で求めます。後者は完全マッチングの各辺を縮め、縮めた木を数えます。どちらも `limit+1` 要素を返し、0頂点の個数は0です。$L=$ `limit` とすると $O(L\log(L+1))$ 時間、$O(L+1)$ 領域を使います。

[使用例](../Examples/labeled_tree_matching_counts_example.cpp) ／ [Verify](../Verify/current/submissions/labeled_tree_matching_counts.cpp)
