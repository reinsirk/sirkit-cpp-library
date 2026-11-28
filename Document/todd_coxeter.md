# Todd–Coxeter法による有限モノイドの列挙

`todd_coxeter(k, relations, limit)` は、生成元と関係式からモノイドの元を列挙し、各生成元を右から掛けた結果を返します。生成元の番号は `0..k-1` です。`MonoidRelation{left, right}` に同じ元を表す2つの語を渡します。空の語は単位元です。

- `size()` は元の個数で、単位元の番号は0です。
- `transition(v, g)` は元 `v` に生成元 `g` を右から掛けた元を返します。
- `evaluate(word)` は語の表す元の番号を返します。2つの語を評価して同じ番号なら、関係式で互いに変形できます。

```cpp
std::array<sirkit::MonoidRelation, 1> relations{{{{0, 0, 0}, {}}}};
auto graph = sirkit::todd_coxeter(1, relations, 10);
// 関係式 a³ = 1 の巡回群。graph->size() は3。
```

仮頂点数が `limit` を超えると `nullopt` を返します。最終的な元が少なくても、等しい元を統合する前に多くの仮頂点を作る場合があります。生成元数を $k$、仮頂点数を $P$、関係式の両辺の総長を $L$、関係式数を $R$ とすると、時間は $O(P(1+k+L+R)\alpha(P))$、追加領域は $O(P(1+k))$ です。

[使用例](../Examples/todd_coxeter_example.cpp) ／ [ローカルVerify](../Verify/current/submissions/todd_coxeter_reference.cpp) ／ [アルゴリズムの解説](https://info.atcoder.jp/entry/algorithm_lectures/todd_coxeter)
