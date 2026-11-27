# 根付き木の最小転倒数

親が子より先に現れる順序のうち、先行する頂点 `u` と後続の頂点 `v` に対する `d[u] * c[v]` の総和を最小にします。

```cpp
const std::array parent{-1, 0, 0};
const std::array<std::uint64_t, 3> c{0, 2, 5}, d{0, 1, 1};
const auto best = sirkit::minimum_inversion_topological_order(3, parent, c, d);
// best.inversions == 2、best.order == {0, 2, 1}。
```

## `minimum_inversion_topological_order`

```text
MinimumInversionTopologicalOrder minimum_inversion_topological_order(
    int n, std::span<const int> parent, std::span<const std::uint64_t> c, std::span<const std::uint64_t> d);
```

`parent`、`c`、`d` はそれぞれ長さ `n` とし、0始まりの頂点番号で指定します。根の `parent` には `-1` を入れます。戻り値の `inversions` は最小値、`order` はその値を達成する頂点順です。空の入力には値0と空の順序を返します。`c`・`d` それぞれの総和や最小値が `std::uint64_t` を超えると `std::overflow_error` になります。

重みの比 `sum(c) / sum(d)` が最大の部分を、親を含む部分の直後にまとめます。$O(n\log(n+1))$ 時間、$O(n)$ 領域です。

[使用例](../Examples/tree_topological_order_minimum_inversions_example.cpp) ／ [Verify](../Verify/current/submissions/rooted_tree_topological_order_with_minimum_inversions.cpp)
