# 永続重み付きUnion-Find

頂点どうしの値の差を制約として追加します。バージョンを保存しておけば、過去の状態を参照し、そこから別の制約を追加できます。[本体](../DataStructure/Persistent_Weighted_Union_Find.cpp)

| 操作 | 内容 |
| --- | --- |
| `version()`, `restore(v)` | 現在のバージョンを取得、または指定のバージョンへ戻す。初期値は `nullptr` |
| `unite_checked(a, b, d[, v])` | `b - a = d` を追加し、`Merged`、`AlreadyConsistent`、`Contradiction` のいずれかを返す |
| `try_diff(a, b[, v])` | `b - a` を取得。別成分なら `std::nullopt` |
| `root(i[, v])`, `root_with_weight(i[, v])` | 代表の頂点番号、または代表とそこからの差 |
| `size(i[, v])`, `issame(a, b[, v])` | 成分の頂点数、同じ成分かどうか。`same` も使用可 |

`v` を省くと現在のバージョンが操作の対象です。`Merged` では新しいバージョンに進み、他の2つでは操作元が現在のバージョンになります。`Version` は保存した状態へのハンドルで、同じ保存領域を共有するインスタンスが残る間有効です。

```cpp
sirkit::PersistentWeightedUnionFind<long long> uf(4);
uf.unite_checked(0, 1, 4);
const auto first = uf.version();
uf.unite_checked(1, 2, -2);
assert(uf.try_diff(0, 2) == 2);
assert(!uf.try_diff(0, 2, first));
```

バージョンの取得・復元は $O(1)$、表中の他の操作は $O(\log^2(n+1))$ です。結合ごとの追加領域は $O(\log(n+1))$ です。

符号付き整数では、中間結果が型の範囲を超えると `std::overflow_error` が発生します。任意の法で計算するには、加算・減算と零をコンストラクタに渡してください。

[実行できる使用例](../Examples/persistent_weighted_union_find_example.cpp)・[AtCoder Verify](../Verify/current/submissions/persistent_weighted_union_find_branching.cpp)
