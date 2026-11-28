# ロールバック付き遅延セグメント木

区間更新と区間の集約を行い、保存した位置まで変更を取り消せます。照会で履歴は増えません。[本体](../DataStructure/Rollback_Lazy_Segment_Tree.cpp)

| 操作 | 内容 | 時間 |
| --- | --- | --- |
| `snapshot()` / `rollback(s)` | 現在の履歴位置を取得、またはその位置へ戻す | $O(1)$ / $O(1+k)$ |
| `apply(l, r, f)` / `prod(l, r)` | 半開区間に作用をかける、または区間を集約する | $O(\log(n+1))$ |
| `set(i, x)` / `get(i)` | 1点を代入、または取得する | $O(\log(n+1))$ |
| `all_prod()` | 全体を集約する | $O(1)$ |

$k$ は取り消すノードの変更件数です。更新は $O(\log(n+1))$ 件を履歴に足します。戻して破棄した先の位置は使えません。`clear_history()` は現在の値を保ち、履歴をすべて破棄します。

`op` と `identity` に集約と単位元を渡します。`mapping(f, value, length)` は長さ `length` の区間の集約に作用を適用し、`composition(f, g)` は `g` の後に `f` を適用する作用を返します。

```cpp
const auto mark = tree.snapshot();
tree.apply(1, 4, 5);  // 区間加算・区間和用の設定
const auto sum_after = tree.prod(0, 4);
tree.rollback(mark);
```

[実行できる使用例](../Examples/rollback_lazy_segment_tree_example.cpp)・[yukicoder Verify](../Verify/current/submissions/rollback_lazy_segment_tree.cpp)
