# 区間mexと出現回数

mexは区間に含まれない最小の非負整数です。指定した区間の値を一括で求めたり、非空区間すべてのmexを値ごとに数えたりできます。

## `range_mex`

```text
std::vector<std::size_t> range_mex(std::span<const std::int64_t> values,
                                   std::span<const RangeMexQuery> queries)
```

`RangeMexQuery{left, right}` は0始まりの半開区間 `[left, right)` で、`left <= right <= values.size()` が前提です。照会を受け取った順にmexを返し、空区間の答えは0です。負の要素は無視します。

配列長を $N$、照会数を $Q$、最長の照会区間の長さを $K$ として、$O((N+Q)\log(K+2))$ 時間・$O(N+Q)$ 領域です。

## `subarray_mex_histogram`

```text
std::vector<std::uint64_t> subarray_mex_histogram(std::span<const std::int64_t> values)
```

結果の添字 `k` に、mexが `k` になる非空区間の数が入ります。結果は長さ `N+1` で、負の要素は無視します。$O((N+1)\log(N+2))$ 時間・$O(N+1)$ 領域です。

## 使用例

```cpp
const std::array<std::int64_t, 4> values{0, 2, 1, 0};
const std::array queries{sirkit::RangeMexQuery{1, 3}, sirkit::RangeMexQuery{0, 3}};
const auto mex = sirkit::range_mex(values, queries);           // {0, 3}
const auto histogram = sirkit::subarray_mex_histogram(values); // {3, 3, 1, 3, 0}
```

[実装](../Algorithm/Range_Mex.cpp)・[使用例の全体](../Examples/range_mex_example.cpp)
