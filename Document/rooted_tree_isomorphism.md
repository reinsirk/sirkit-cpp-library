# 根付き部分木の同型判定

各頂点を根とする部分木に番号を振ります。子の並び順を変えて一致する木には同じ番号が付きます。

```cpp
std::vector<int> parent{-1, 0, 0};
auto result = sirkit::rooted_subtree_classes(parent);
```

## `rooted_subtree_classes`

```text
RootedSubtreeClasses rooted_subtree_classes(std::span<const int> parent);
```

添字を頂点番号とし、各頂点について親番号、根には `-1` を指定します。複数の根を持つ森も扱えます。戻り値の `count` は種類数、`class_id[v]` は頂点 $v$ の分類番号です。分類番号は呼び出し内で比較できます。$O(n\log(n+1))$ 時間・$O(n)$ 領域です。

[使用例](../Examples/rooted_tree_isomorphism_example.cpp) ／ [Verify](../Verify/current/submissions/rooted_tree_isomorphism_classification.cpp)
