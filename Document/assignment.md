# 最小費用割当

行ごとに異なる列を選び、選んだ要素の合計を最小化します。負の費用と横長の行列にも対応します。

```cpp
std::vector<std::vector<std::int64_t>> cost{{8, 1, 4}, {2, 5, 3}};
auto result = sirkit::minimum_assignment(cost);
```

## `minimum_assignment`

```text
AssignmentResult minimum_assignment(std::span<const std::vector<std::int64_t>> cost);
```

行数 $n$ 以下でない列数 $m$ を指定します。戻り値の `cost` は最小の合計費用、`column[i]` は行 $i$ に割り当てた列です。空行列の結果は費用0・空の割当です。各費用の絶対値は `INT64_MAX/(2*n+3)` 以下です。$O(n^2m)$ 時間・$O(n+m)$ 追加領域です。

[使用例](../Examples/assignment_example.cpp) ／ [Verify](../Verify/current/submissions/assignment.cpp)
