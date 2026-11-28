# 固定列の二分探索

同じ数列に対して下限・上限を繰り返し探します。値は内部で二分探索木の幅優先順に保持し、結果は通常のソート順での添字を返します。

| API | 内容 |
| --- | --- |
| `EytzingerSearch(values[, compare])` | ソートして構築する。$O(n\log(n+1))$ 時間。 |
| `EytzingerSearch<T>::from_sorted(values)` | 整列済みの列で構築する。$O(n)$ 時間。 |
| `lower_bound(key)` / `upper_bound(key)` | `key` 以上 / より大きい最初の位置。該当しなければ `size()`。 |
| `equal_range(key)` | `key` と同順位の要素が並ぶ半開区間。 |

探索は $O(\log(n+1))$ 時間、構築と保持に使う領域は $O(n)$ です。比較器を指定した場合は、その順序に従います。実行時間は配列サイズとキャッシュの容量によって変わります。

```cpp
const sirkit::EytzingerSearch data(std::vector{3, 1, 3, 7});
const auto [left, right] = data.equal_range(3); // [1,3)
```

[使用例](../Examples/eytzinger_search_example.cpp) ／ [Verify](../Verify/current/submissions/eytzinger_search.cpp) ／ [配列配置の論文](https://arxiv.org/abs/1509.05053)
