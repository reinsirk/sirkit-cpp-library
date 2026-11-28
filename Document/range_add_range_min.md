# 区間加算と区間最小値

区間の各要素に同じ整数を加え、その後の区間最小値を照会します。[本体](../DataStructure/Range_Add_Range_Min.cpp)

| 操作 | 内容 | 時間 |
| --- | --- | --- |
| `RangeAddRangeMin<T>(values)` / `(n)` | 初期配列、または `n` 個の0から構築 | $O(n)$ |
| `add(l, r, d)` | 半開区間 `[l, r)` に `d` を加える | $O(\log(n+1))$ |
| `minimum(l, r)` | 区間最小値。空区間なら `std::nullopt` | $O(\log(n+1))$ |
| `all_minimum()` | 全体の最小値 | $O(1)$ |

```cpp
const std::array<long long, 4> values{3, 1, 4, 2};
sirkit::RangeAddRangeMin<long long> tree(values);
tree.add(1, 3, 5);
assert(tree.minimum(0, 3) == 3);
```

領域は $O(n)$ です。添字は0始まりで、`T` は符号付き整数型です。保留中の加算の合計と計算途中の値も型の範囲に収めてください。

[実行できる使用例](../Examples/range_add_range_min_example.cpp)・[Library Checker Verify](../Verify/current/submissions/range_add_range_min.cpp)
