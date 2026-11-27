# 区間に含まれる値の種類数

固定された数列と区間の一覧から、各区間に何種類の値が現れるかをまとめて求めます。

```cpp
const std::vector<std::int64_t> values{5, -2, 5, 3, -2};
const std::vector<sirkit::DistinctCountQuery> queries{{0, 5}, {1, 3}, {3, 3}};
const auto counts = sirkit::offline_distinct_count(values, queries); // {3, 2, 0}
```

## `offline_distinct_count`

```text
std::vector<std::size_t> offline_distinct_count(std::span<const std::int64_t> values,
                                               std::span<const DistinctCountQuery> queries);
```

クエリは `DistinctCountQuery{left, right}` で渡します。区間は0始まりで、左端を含み右端を含まない `[left, right)` です。種類数をクエリと同じ順で返します。空区間の種類数は0です。

`0 <= left <= right <= values.size()` を満たしてください。検査有効時は範囲外の値に `std::out_of_range` を送出します。数列には符号付き64ビット整数の全域を使えます。

右端の小さい区間から処理し、各値の最後の出現位置だけを Fenwick Tree で数えます。数列長を `N`、クエリ数を `Q` として、時間は `O((N+Q) log(N+2))`、領域は `O(N+Q)` です。

[使用例](../Examples/offline_distinct_count_example.cpp) ／ [Verify](../Verify/current/submissions/offline_distinct_count.cpp) ／ [AtCoder の公式解説（F）](https://img.atcoder.jp/abc174/editorial.pdf)
